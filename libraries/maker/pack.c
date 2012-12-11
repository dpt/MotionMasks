/* pack.c */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "base/mmerror.h"
#include "base/types.h"
#include "base/debug.h"

#include "framebuf/bitmap-set.h"

#include "utils/array.h"

#include "maker/make.h"

#include "mm/types.h"
#include "mm/codes.h"

#include "impl.h"

/* -------------------------------------------------------------------------- */

#define INITIAL_DATA 65536

/* -------------------------------------------------------------------------- */

typedef unsigned int mmscanlinehash_t;

/* -------------------------------------------------------------------------- */

static mmscanlinehash_t hash_row_y8(const void *base, int width)
{
  const unsigned char *s = base;
  mmscanlinehash_t     h;

  h = 0x811c9dc5;
  while (width--)
  {
    h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24);
    h ^= *s++;
  }

  return h;
}

/* -------------------------------------------------------------------------- */

typedef struct mmscanlineinfo
{
  int              bitmap;
  int              scanline;
  mmscanlinehash_t hash;
  int              prime; // scanline index we're a duplicate of
  mmdata_t        *encoded;
  size_t           nencoded; // length of encoded data
}
mmscanlineinfo_t;

/* -------------------------------------------------------------------------- */

static const mmscanlineinfo_t *sort_opaque;

static int infocompare(const void *via, const void *vib)
{
  const mmscanlineinfo_t *info = sort_opaque;
  const int              *ia = via;
  const int              *ib = vib;
  const mmscanlineinfo_t *a = &info[*ia];
  const mmscanlineinfo_t *b = &info[*ib];

  if (a->hash < b->hash)
    return -1;
  if (a->hash > b->hash)
    return +1;

  if (a->bitmap < b->bitmap)
    return -1;
  if (a->bitmap > b->bitmap)
    return +1;

  if (a->scanline < b->scanline)
    return -1;
  if (a->scanline > b->scanline)
    return +1;

  //sentinel(); // <- needs a failure: label to take
  assert("Can't get here.");

  return 0;
}

/* -------------------------------------------------------------------------- */

mmerror_t motionmaskmaker_pack(motionmaskmaker_t  *maker,
                               const bitmap_set_t *bitmaps)
{
  mmerror_t         err;

  mmscanlineinfo_t *info = NULL;
  int               infoused;
  int               infoallocated;

  mmdata_t         *data = NULL;
  int               dataused;
  int               dataallocated;

  int               noffsets;
  mmoffset_t       *offsets = NULL;
  int               offsetsused;

  int               nframes;
  mmframeindex_t   *frames = NULL;
  int               framesused;

  mmdata_t         *datap;
  int               bm;
  const uint8_t    *base;
  int               y;
  int              *indices = NULL;
  int               i;
  int               ndupes;

  if (bitmaps->width  == 0 ||
      bitmaps->height == 0 ||
      bitmaps->nbases == 0)
    return mmerror_BAD_ARG; /* no input bitmaps specified */

  /* populate 'info' table with scanline info for all scanlines */

  info          = NULL;
  infoused      = 0;
  infoallocated = 0;

  for (bm = 0; bm < bitmaps->nbases; bm++)
  {
    base = bitmaps->bases[bm];

    for (y = 0; y < bitmaps->height; y++)
    {
      mmscanlineinfo_t infoentry;

      infoentry.bitmap   = bm;
      infoentry.scanline = y;
      infoentry.hash     = hash_row_y8(base, bitmaps->width);
      infoentry.prime    = -1;
      infoentry.encoded  = NULL;
      infoentry.nencoded = 0;

      if (array_grow((void **) &info,
                     sizeof(*info),
                     infoused,
                     &infoallocated,
                     1,
                     1))
        goto oom;

      info[infoused++] = infoentry;

      base += bitmaps->rowbytes;
    }
  }

  /* create linear index */

  indices = malloc(sizeof(*indices) * infoused);
  if (indices == NULL)
    goto oom;

  for (i = 0; i < infoused; i++)
    indices[i] = i;

  /* sort the index by hash */

  sort_opaque = info; // must pass via static since qsort takes no opaque value
  qsort(indices, infoused, sizeof(*indices), infocompare);

  /* walk pairs of sorted indices and mark duplicates */

  for (i = 0; i < infoused - 1; )
  {
    int                     ia, ib;
    const mmscanlineinfo_t *a;
    mmscanlineinfo_t       *b;

    ia = i + 0;
    ib = i + 1;

    a = &info[indices[ia]];

    assert(a->prime == -1); /* we ought to never hit a dupe here */

    ndupes = 1;

    while (ib < infoused)
    {
      const uint8_t *baseA;
      const uint8_t *baseB;

      b = &info[indices[ib]];

      if (a->hash != b->hash)
        break;

      baseA = bitmaps->bases[a->bitmap];
      baseA += a->scanline * bitmaps->rowbytes;

      baseB = bitmaps->bases[b->bitmap];
      baseB += b->scanline * bitmaps->rowbytes;

      if (memcmp(baseA, baseB, bitmaps->rowbytes) != 0)
      {
        debugf("memcmp didn't agree with hash");
        break;
      }

      ndupes++;

      b->prime = indices[ia];

      ib++;
    }

    if (ndupes > 1)
      debugf("%d: %d duplicate scanlines", i, ndupes);

    i = ib;
  }

  free(indices);
  indices = NULL;

  /* now pack the scanlines */

  data             = malloc(INITIAL_DATA);
  dataused         = 0;
  dataallocated    = INITIAL_DATA;

  noffsets         = bitmaps->nbases * bitmaps->height;
  offsets          = malloc(noffsets * sizeof(*offsets));
  offsetsused      = 0;

  nframes          = bitmaps->nbases;
  frames           = malloc(nframes * sizeof(*frames));
  framesused       = 0;

  if (data == NULL || offsets == NULL || frames == NULL)
    goto oom;

  datap = data;

  for (bm = 0; bm < bitmaps->nbases; bm++)
  {
    int yy;

    yy = bm * bitmaps->height;  /* input frames all have identical dimensions currently */
    frames[framesused++] = yy;

    base = bitmaps->bases[bm];

    for (y = 0; y < bitmaps->height; y++)
    {
      mmscanlineinfo_t *sli;

      sli = &info[yy + y];
      if (sli->prime >= 0)
        sli = &info[sli->prime];

      assert(sli->prime == -1); /* must be a prime */

      if (sli->encoded == NULL && sli->nencoded == 0)
      {
        size_t used;

        err = encode_row_y8(base,
                            bitmaps->width,
                            datap,
                            dataallocated - dataused,
                            &used);
        if (err == mmerror_BUFFER_OVERFLOW)
        {
          debugf("Out of buffer at frame %d, row %d (used=%d)\n",
                 bm, y, dataused);
        }
        if (err)
          goto failure;

        assert(datap[used - 1] == MMStop_VAL);

        sli->encoded  = datap;
        sli->nencoded = used;

        datap    += used;
        dataused += used;
      }

      offsets[offsetsused++] = sli->encoded;

      base += bitmaps->rowbytes;
    }
  }

  free(info);
  info = NULL;

  maker->data     = data;
  maker->ndata    = dataused;

  maker->offsets  = offsets;
  maker->noffsets = offsetsused;

  maker->frames   = frames;
  maker->nframes  = framesused;

  maker->width    = bitmaps->width;
  maker->height   = bitmaps->height;

  logf_info("motionmaskmaker_pack: data %zd long", dataused);
  logf_info("motionmaskmaker_pack: offsets %zd long", offsetsused);

  return mmerror_OK;


oom:

  err = mmerror_OOM;

failure:

  free(data);
  free(offsets);
  free(frames);
  free(info);

  return err;
}
