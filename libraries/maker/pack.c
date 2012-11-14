/* pack.c */

#include <stdlib.h>

#include "base/error.h"
#include "base/types.h"
#include "base/debug.h"

#include "framebuf/bitmap-set.h"

#include "maker/make.h"

#include "mmtypes.h"

#include "impl.h"

#define INITIAL_DATA 65536

mmerror_t motionmaskmaker_pack(motionmaskmaker_t  *maker,
                               const bitmap_set_t *bitmaps)
{
  mmerror_t     err;

  data_t       *data;
  int           dataused;
  int           dataallocated;

  int           noffsets;
  offset_t     *offsets;
  int           offsetsused;

  int           nframes;
  frameindex_t *frames;
  int           framesused;

  data_t       *datap;

  int           bm;

  // ensure all bitmaps are same size, format, etc.

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
    const uint8_t *base;
    int            y;

    base = bitmaps->bases[bm];

    frames[bm] = bm * bitmaps->height;
    framesused++;

    for (y = 0; y < bitmaps->height; y++)
    {
      int used;

      offsets[y] = datap;
      offsetsused++;

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

      base     += bitmaps->rowbytes;
      datap    += used;
      dataused += used;
    }
  }

  maker->data     = data;
  maker->ndata    = dataused;

  maker->offsets  = offsets;
  maker->noffsets = offsetsused;

  maker->frames   = frames;
  maker->nframes  = framesused;


  maker->width    = bitmaps->width;
  maker->height   = bitmaps->height;

  return mmerror_OK;


oom:

  err = mmerror_OOM;

failure:

  free(data);
  free(offsets);
  free(frames);

  return err;
}
