/* rle.c */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/debug.h"
#include "base/mmerror.h"
#include "base/types.h"

#include "framebuf/pixelfmt.h"
#include "mm/codes.h"
#include "utils/array.h"
#include "utils/minmax.h"

#include "mm/types.h"

#include "impl.h"

/* ----------------------------------------------------------------------- */

typedef struct encstate
{
  mmdata_t *dst;
  mmdata_t *dstend;

  struct
  {
    int     hits;
    int     pixels;
  }
  stats[MMID_LIMIT];
}
encstate_t;

/* ----------------------------------------------------------------------- */

static mmerror_t emit(encstate_t *state, uint32_t code, int codelength)
{
  if (state->dst == state->dstend)
    return mmerror_BUFFER_OVERFLOW;

  while (codelength--)
    *state->dst++ = (code >> (codelength * 8)) & 0xFF;

  return mmerror_OK;
}

/* ----------------------------------------------------------------------- */

static mmerror_t emit_copy(encstate_t *state, int n, int source)
{
  mmerror_t err;

  assert(source == 0 || source == 1);

  while (n > 0)
  {
    int      length;
    uint32_t code;
    int      codelength;

    length = MIN(MMCopyLong_LENGTH_MAX, n);
    if (length <= MMCopy_LENGTH_MAX)
    {
      code = MMCopy_VAL |
             (source << MMCopy_SOURCE_SHIFT) |
             (length & MMCopy_LENGTH_MASK);
      codelength = 1;

      state->stats[MMCopy_ID].hits++;
      state->stats[MMCopy_ID].pixels += length;
    }
    else
    {
      /* CopyLong encodes runs 2^6+ pixels long */

      code = (MMCopyLong_VAL << 8) |
             (source << MMCopyLong_SOURCE_SHIFT) |
             ((length - MMCopyLong_LENGTH_MIN) & MMCopyLong_LENGTH_MASK);
      codelength = 2;

      state->stats[MMCopyLong_ID].hits++;
      state->stats[MMCopyLong_ID].pixels += length;
    }

    err = emit(state, code, codelength);
    if (err)
      return err;

    n -= length;
  }

  return mmerror_OK;
}

static mmerror_t emit_blendconst(encstate_t *state, int n, mmalpha_t alpha)
{
  mmerror_t err;

  while (n > 0)
  {
    int      length;
    uint32_t code;
    int      codelength;

    length = MIN(MMBlendConstLong_LENGTH_MAX, n);
    if (length <= MMBlendConst_LENGTH_MAX)
    {
      code = MMBlendConst_VAL |
             (length & MMBlendConst_LENGTH_MASK);
      codelength = 1;

      state->stats[MMBlendConst_ID].hits++;
      state->stats[MMBlendConst_ID].pixels += length;
    }
    else
    {
      /* BlendConstLong encodes runs 2^6+ pixels long */

      code = (MMBlendConstLong_VAL << 8) |
             ((length - MMBlendConstLong_LENGTH_MIN) & MMBlendConstLong_LENGTH_MASK);
      codelength = 2;

      state->stats[MMBlendConstLong_ID].hits++;
      state->stats[MMBlendConstLong_ID].pixels += length;
    }

    err = emit(state, (code << 8) | alpha, codelength + 1);
    if (err)
      return err;

    n -= length;
  }

  return mmerror_OK;
}

static mmerror_t emit_blendarray(encstate_t      *state,
                                 int              n,
                                 const mmalpha_t *alphas)
{
  mmerror_t err;

  while (n > 0)
  {
    int            length;
    uint32_t       code;
    int            codelength;
    const uint8_t *end;

    length = MIN(MMBlendArrayLong_LENGTH_MAX, n);
    if (length <= MMBlendArray_LENGTH_MAX)
    {
      code = MMBlendArray_VAL |
             (length & (MMBlendArray_LENGTH_MAX - 1));
      codelength = 1;

      state->stats[MMBlendArray_ID].hits++;
      state->stats[MMBlendArray_ID].pixels += length;
    }
    else
    {
      /* BlendArrayLong encodes runs 2^5+ pixels long */

      code = (MMBlendArrayLong_VAL << 8) |
             ((length - MMBlendArrayLong_LENGTH_MIN) & MMBlendArray_LENGTH_MASK);
      codelength = 2;

      state->stats[MMBlendArrayLong_ID].hits++;
      state->stats[MMBlendArrayLong_ID].pixels += length;
    }

    err = emit(state, code, codelength);
    if (err)
      return err;

    for (end = alphas + length; alphas < end; alphas++)
      emit(state, *alphas, 1);

    n -= length;
  }

  return mmerror_OK;
}

static mmerror_t emit_stop(encstate_t *state)
{
  state->stats[MMStop_ID].hits++;

  return emit(state, MMStop_VAL, 1);
}

/* ----------------------------------------------------------------------- */

mmerror_t encode_row_y8(const void *vsrc,
                        int         nsrcpix,
                        uint8_t    *dst,
                        size_t      ndstbytes,
                        size_t     *dstused)
{
  mmerror_t      err;
  encstate_t     state;
  int            i;
  const uint8_t *src = vsrc;
  const uint8_t *end = src + nsrcpix;

  state.dst    = dst;
  state.dstend = dst + ndstbytes;

  for (i = 0; i < MMID_LIMIT; i++)
  {
    state.stats[i].hits   = 0;
    state.stats[i].pixels = 0;
  }

  do
  {
    const uint8_t *repeats_start,  *repeats_end;
    const uint8_t *literals_start, *literals_end;

    /* find the longest string of identical input bytes */

    repeats_start = src;
    while (src + 1 < end && src[0] == src[1])
      src++;
    if (src == repeats_start)
      repeats_end = src; /* no repetitions: empty run */
    else
      repeats_end = ++src; /* exclusive upper bound */

    /* find the longest string of non-identical input bytes */

    literals_start = src;
    while (src + 1 < end && src[0] != src[1])
      src++;
    if (src + 1 == end) /* fold any trailing singles into the literal */
      src++;
    literals_end = src; /* exclusive upper bound */

    if (repeats_start != repeats_end)
    {
      int first;

      first = *repeats_start;

      if (first == 0x00 || first == 0xFF)
      {
        /* constant opacity: copy */
        err = emit_copy(&state,
                        (int) (repeats_end - repeats_start),
                        first == 0xFF);
        if (err)
          return err;
      }
      else
      {
        /* constant transparency: blend const */
        err = emit_blendconst(&state,
                              (int) (repeats_end - repeats_start),
                              (mmalpha_t) first);
        if (err)
          return err;
      }
    }

    if (literals_start != literals_end)
    {
      /* variable transparency: blend array */
      err = emit_blendarray(&state,
                            (int) (literals_end - literals_start),
                            literals_start);
      if (err)
        return err;
    }
  }
  while (src < end);

  emit_stop(&state);

  if (dstused)
    *dstused = state.dst - dst;

  return mmerror_OK;
}

/*
int encodetest(void)
{
  static const uint8_t scanline[] = { 0,0,0,0,1,2,0,255,3,4,4,4,4,255,255,255 };

  mmerror_t err;
  uint8_t   buf[100];
  int       used;
  int       i;

  err = encode_row_y8(scanline, NELEMS(scanline), buf, NELEMS(buf), &used);
  if (err)
    return 1;

  for (i = 0; i < used; i++)
    printf("%.2x ", buf[i]);
  printf("\n");

  return 0;
}
*/
