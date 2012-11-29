/* plot.c */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "base/debug.h"
#include "base/mmerror.h"
#include "base/suppress.h"

#include "mm/types.h"
#include "mm/codes.h"
#include "framebuf/pixelfmt.h"
#include "plot/span-registry.h"
#include "utils/barith.h"
#include "utils/likely.h"
#include "utils/minmax.h"

#include "player/play.h"

#include "impl.h"

// todo
// - style mapping
// - source images of different dimensions
// - increment only the used source image pointers [done]

/* ----------------------------------------------------------------------- */

#define MMDEBUG 0

/* Largest possible number of surfaces - used to size arrays. */
#define MAXSURFACES 3 /* 0: screen, 1: source_A, 2: source_B */

/* ----------------------------------------------------------------------- */

/* A source image index. Holds up to MAXSOURCES. */
typedef uint8_t source_t;

/* ----------------------------------------------------------------------- */

typedef struct state
{
  const span_t    *span;
  unsigned char   *surfaces[MAXSURFACES];
  size_t           log2bpp; /* bytes (not bits) per pixel for all surfaces */
  int              skip; /* number of pixels to skip at the start of the row */
  int              plot; /* number of pixels to plot */
  source_t         sourceindexes[MAXSOURCES];
  const bitmap_t **sources;
  int              nsources;
  int              y;
}
state_t;

/* ----------------------------------------------------------------------- */

// note on skip/plot counts:
// say:
// length = 4
// skip = 2
// skippable = MIN(4, 2) = 2
// skip -= 2
// plot = 3
// plottable = MIN(4 - 2, 3) = 2
// plot -= 2
// => skip = 0, plot = 1

static void copy(int source, int length, state_t *state)
{
  int skip;
  int plot;

  skip = MIN(length, state->skip); /* in pixels */
  state->skip -= skip;

  plot = MIN(length - skip, state->plot); /* in pixels */
  state->plot -= plot;

  // screen surface points to the next pixel to be written
  // source surface points to the next pixel to be read
  // both take into account the left-skip
  if (plot <= 0)
    return;

  state->span->copy(state->surfaces[0], state->surfaces[source + 1], plot);

  plot <<= state->log2bpp;
  state->surfaces[0] += plot;
  state->surfaces[1] += plot;
  state->surfaces[2] += plot;
}

static const mmdata_t *decode_copy(unsigned int  code,
                                   const mmdata_t *p,
                                   state_t      *state)
{
  int source;
  int length; /* in pixels */

  source = (code & MMCopy_SOURCE_MASK) >> MMCopy_SOURCE_SHIFT;

  length = (code & MMCopy_LENGTH_MASK) >> MMCopy_LENGTH_SHIFT;
  length = ZEROIS2N(length, MMCopy_LENGTH_BITS);

  if (MMDEBUG)
    debugf("decode_copy: source %d, length %d", source, length);

  copy(source, length, state);

  return p;
}

static const mmdata_t *decode_copylong(unsigned int  code,
                                       const mmdata_t *p,
                                       state_t      *state)
{
  int source;
  int length; /* in pixels */

  code = (code << 8) | *p++;

  source = (code & MMCopyLong_SOURCE_MASK) >> MMCopyLong_SOURCE_SHIFT;

  length = (code & MMCopyLong_LENGTH_MASK) >> MMCopyLong_LENGTH_SHIFT;
  length = ZEROIS2N(length, MMCopyLong_LENGTH_BITS);

  if (MMDEBUG)
    debugf("decode_copylong: source %d, length %d", source, length);

  copy(source, length, state);

  return p;
}

/* ----------------------------------------------------------------------- */

static void blendconst(int length, int alpha, state_t *state)
{
  int skip;
  int plot;

  skip = MIN(length, state->skip); /* in pixels */
  state->skip -= skip;

  plot = MIN(length - skip, state->plot); /* in pixels */
  state->plot -= plot;

  if (plot <= 0)
    return;

  state->span->blendconst(state->surfaces[0],
                          state->surfaces[1],
                          state->surfaces[2],
                          plot,
                          alpha);

  plot <<= state->log2bpp;
  state->surfaces[0] += plot;
  state->surfaces[1] += plot;
  state->surfaces[2] += plot;
}

static const mmdata_t *decode_blendconst(unsigned int  code,
                                         const mmdata_t *p,
                                         state_t      *state)
{
  int length;
  int alpha; // not alpha_t, widened

  length = (code & MMBlendConst_LENGTH_MASK) >> MMBlendConst_LENGTH_SHIFT;
  length = ZEROIS2N(length, MMBlendConst_LENGTH_BITS);

  alpha  = *p++;

  if (MMDEBUG)
    debugf("decode_blendconst: length %d, alpha %d", length, alpha);

  blendconst(length, alpha, state);

  return p;
}

static const mmdata_t *decode_blendconstlong(unsigned int  code,
                                             const mmdata_t *p,
                                             state_t      *state)
{
  int length;
  int alpha; // not alpha_t, widened

  code = (code << 8) | *p++;

  length = (code & MMBlendConstLong_LENGTH_MASK) >> MMBlendConstLong_LENGTH_SHIFT;
  length = ZEROIS2N(length, MMBlendConstLong_LENGTH_BITS);

  alpha  = *p++;

  if (MMDEBUG)
    debugf("decode_blendconstlong: length %d, alpha %d", length, alpha);

  blendconst(length, alpha, state);

  return p;
}

/* ----------------------------------------------------------------------- */

static void blendarray(int length, const mmalpha_t *palphas, state_t *state)
{
  int skip;
  int plot;

  skip = MIN(length, state->skip); /* in pixels */
  state->skip -= skip;

  plot = MIN(length - skip, state->plot); /* in pixels */
  state->plot -= plot;

  if (plot <= 0)
    return;

  state->span->blendarray(state->surfaces[0],
                          state->surfaces[1],
                          state->surfaces[2],
                          plot,
                          palphas + skip);

  plot <<= state->log2bpp;
  state->surfaces[0] += plot;
  state->surfaces[1] += plot;
  state->surfaces[2] += plot;
}

static const mmdata_t *decode_blendarray(unsigned int  code,
                                         const mmdata_t *p,
                                         state_t      *state)
{
  int              length;
  const mmalpha_t *palphas;

  length  = (code & MMBlendArray_LENGTH_MASK) >> MMBlendArray_LENGTH_SHIFT;
  length  = ZEROIS2N(length, MMBlendArray_LENGTH_BITS);

  palphas = p;

  if (MMDEBUG)
    debugf("decode_blendarray: length %d", length);

  blendarray(length, palphas, state);

  return p + length; /* skip over alphas */
}

static const mmdata_t *decode_blendarraylong(unsigned int    code,
                                             const mmdata_t *p,
                                             state_t        *state)
{
  int              length;
  const mmalpha_t *palphas;

  code = (code << 8) | *p++;

  length  = (code & MMBlendArrayLong_LENGTH_MASK) >> MMBlendArrayLong_LENGTH_SHIFT;
  length  = ZEROIS2N(length, MMBlendArrayLong_LENGTH_BITS);

  palphas = p;

  if (MMDEBUG)
    debugf("decode_blendarraylong: length %d", length);

  blendarray(length, palphas, state);

  return p + length; /* skip over alphas */
}

/* ----------------------------------------------------------------------- */

static const mmdata_t *decode_unknown(unsigned int    code,
                                      const mmdata_t *p,
                                      state_t        *state)
{
  NOT_USED(state);

  if (MMDEBUG)
    logf_error("Unknown control code '%d' encountered.", code);

  return p;
}

/* ----------------------------------------------------------------------- */

static void setsource(source_t source1, source_t source2, state_t *state, int force)
{
  source_t  maxsource;
  ptrdiff_t offset;
  int       x, y;
  source_t  s;

  maxsource = (source_t) (state->nsources - 1);

  source1 = CLAMP(source1, 0, maxsource);
  source2 = CLAMP(source2, 0, maxsource);

  if (!force && (state->sourceindexes[0] == source1 &&
                 state->sourceindexes[1] == source2))
  {
    if (MMDEBUG)
      debugf("setsource: sources remain unchanged");
    return; /* no change */
  }

  /* work out current screen coordinates */

  offset = state->surfaces[1] - (uint8_t *) state->sources[1]->base;
  y = state->y;
  x = (int) ((offset - y * state->sources[1]->rowbytes) >> state->log2bpp); // could hoist the log2bpp shifts out

  /* recalculate offsets */

  if (force || state->sourceindexes[0] != source1)
  {
    s = state->sourceindexes[0] = (source_t) source1;
    state->surfaces[1] = (unsigned char *) state->sources[s]->base +
                         y * state->sources[s]->rowbytes +
                         (x << state->log2bpp);
  }
  if (force || state->sourceindexes[1] != source2)
  {
    s = state->sourceindexes[1] = (source_t) source2;
    state->surfaces[2] = (unsigned char *) state->sources[s]->base +
                         y * state->sources[s]->rowbytes +
                         (x << state->log2bpp);
  }
}

static const mmdata_t *decode_setsource(unsigned int    code,
                                        const mmdata_t *p,
                                        state_t        *state)
{
  source_t source1, source2;

  code = *p++;

  source1 = (code & MMSetSource_SOURCE1_MASK) >> MMSetSource_SOURCE1_SHIFT;
  source2 = (code & MMSetSource_SOURCE2_MASK) >> MMSetSource_SOURCE2_SHIFT;

  if (MMDEBUG)
    debugf("decode_setsource: %d, %d", source1, source2);

  setsource(source1, source2, state, 0 /* don't force */);

  return p;
}

/* ----------------------------------------------------------------------- */

typedef const mmdata_t *(*decoder_t)(unsigned int    code,
                                     const mmdata_t *p,
                                     state_t        *state);

/* decode a single row */
static void decode_row(const mmdata_t *data, state_t *state)
{
  static const decoder_t decoders[] =
  {
    decode_copy,
    decode_blendconst,
    decode_blendarray,
    decode_copylong,
    decode_blendconstlong,
    decode_blendarraylong,
    decode_unknown,
    decode_setsource,
  };

  for (;;)
  {
    unsigned int ctl; /* widened */
    int          clz;

    /* read control byte */
    ctl = *data++;
    clz = clz8(ctl);

    if (unlikely(clz == MMStop_ID))
    {
      if (MMDEBUG)
        debugf("%s", "stop");
      break;
    }

    data = decoders[clz](ctl, data, state);

    /* quit if we've finished early */
    if (unlikely(state->plot <= 0))
      break;
  }
}

mmerror_t motionmaskplayer_plot(const motionmaskplayer_t *player,
                                const bitmap_t           *sources[],
                                int                       nsources,
                                const screen_t           *screen,
                                int                       x,
                                int                       y,
                                int                       frameidx)
{
  state_t        state;
  const frame_t *frame;
  box_t          screenbox;
  box_t          areabox;
  unsigned char *surfaces[MAXSURFACES]; // can do [nsources] in C99
  int            i;
  int            row;
  int            rowend;
  int            clipped_width, clipped_height;
  box_t          clippedscreenbox;
  box_t          clippedscreenareabox;
  box_t          screenskip;
  int            imageskip_x0;
  int            imageskip_y0;

  if (unlikely(player == NULL || sources == NULL || screen == NULL))
    return mmerror_BAD_ARG;

  if (unlikely(nsources < 0 || nsources > motionmaskplayer_MAXSOURCES))
  {
    logf_error("Invalid number of source bitmaps (%d)\n", nsources);
    return mmerror_BAD_ARG;
  }

  if (unlikely(frameidx < 0 || frameidx >= player->nframes))
  {
    logf_error("Invalid frame index (%d)\n", frameidx);
    return mmerror_BAD_ARG;
  }

  frame = &player->frames[frameidx];

  /* select appropriate span plotting methods for this screen depth */

  state.span = spanregistry_get(screen->format);
  if (unlikely(state.span == NULL))
  {
    logf_error("Unavailable pixel format (%d).", screen->format);
    return mmerror_BAD_ARG;
  }

  /* check that all surfaces are the same depth as the screen */

  for (i = 0; i < nsources; i++)
  {
    if (unlikely(sources[i]->format != screen->format))
    {
      logf_error("Source pixel format doesn't match screen (source %d).", i);
      return mmerror_BAD_ARG;
    }
  }

  /* clamp inputs to screen space */

  screenbox.x0 = 0;
  screenbox.y0 = 0;
  screenbox.x1 = screen->width;
  screenbox.y1 = screen->height;

  /* intersect screen box with clip box */
  box_intersection(&screenbox, &screen->clip, &clippedscreenbox);
  if (unlikely(box_is_empty(&clippedscreenbox)))
  {
    logf_warning("%s", "Intersection of screen and clip boxes is empty.");
    return mmerror_OK;
  }

  areabox.x0 = x;
  areabox.y0 = y;
  areabox.x1 = x + player->width;
  areabox.y1 = y + player->height;

  /* intersect clipped screen box with motion mask area box */
  box_intersection(&clippedscreenbox, &areabox, &clippedscreenareabox);
  if (unlikely(box_is_empty(&clippedscreenareabox)))
  {
    logf_warning("%s", "Intersection of clipped screen and area boxes is empty.");
    return mmerror_OK;
  }

  /* clipped screen box gives me the screen offsets
   * this is the box of all the pixels we'll write on */
  screenskip = clippedscreenareabox; // taking local copy

  if (MMDEBUG)
    debugf("screen: %d %d %d %d",
           screenskip.x0, screenskip.y0, screenskip.x1, screenskip.y1);

  clipped_width  = screenskip.x1 - screenskip.x0;
  clipped_height = screenskip.y1 - screenskip.y0;

  /* then we can work out how much source image to skip */
  imageskip_x0 = MAX(screenskip.x0 - x, 0);
  imageskip_y0 = MAX(screenskip.y0 - y, 0);

  if (MMDEBUG)
    debugf("image: %d %d", imageskip_x0, imageskip_y0);

  /* calculate log2 BYTES-per-pixel for each surface */

  state.log2bpp = pixelfmt_log2bpp(screen->format) - 3; /* bits -> bytes */

  /* calculate surface destination row addresses, merging destination and
   * source pointers into a single array */

  surfaces[0] = (unsigned char *) screen->base +
                screenskip.y0 * screen->rowbytes +
                (screenskip.x0 << state.log2bpp);

  /* do this only for the current two active surfaces */

  // setup only surface_A as that's all setsource uses to work out x coord from

  surfaces[1] = (unsigned char *) sources[1]->base +
                imageskip_y0 * sources[1]->rowbytes +
                (imageskip_x0 << state.log2bpp);

  state.surfaces[0] = surfaces[0];
  state.surfaces[1] = surfaces[1];
  //state.surfaces[2] = surfaces[2];

  state.y = imageskip_y0;

  // stuff we need for setsource

  state.sources  = sources;
  state.nsources = nsources;

  /* populate source index */
  int fsource = frame->source;
  setsource((fsource >> 0) & 0xF, (fsource >> 4) & 0xF, &state, 1 /* force */);

  // setsource only sets (current surface pointers) state.surfaces, so copy
  surfaces[0] = state.surfaces[0];
  surfaces[1] = state.surfaces[1];
  surfaces[2] = state.surfaces[2];

  // row counters

  rowend = frame->start + imageskip_y0 + clipped_height;

  for (row = frame->start + imageskip_y0; row < rowend; row++)
  {
    state.skip = imageskip_x0; // reset skip counter
    state.plot = clipped_width;

    /* copy surface pointers */
    state.surfaces[0] = surfaces[0];
    state.surfaces[1] = surfaces[1];
    state.surfaces[2] = surfaces[2];

    /* increment surface pointers for next time */
    surfaces[0] += screen->rowbytes;
    surfaces[1] += sources[state.sourceindexes[0]]->rowbytes;
    surfaces[2] += sources[state.sourceindexes[1]]->rowbytes;

    state.y++; // only setsource needs this

    assert(player->offsets[row] >= player->data);
    assert(player->offsets[row] < player->data + player->ndata);

    decode_row(player->offsets[row], &state);
  }

  return mmerror_OK;
}
