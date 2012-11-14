/* save.c */

#include <stdio.h>

#include "base/error.h"
#include "base/debug.h"
#include "base/types.h"

#include "utils/pack.h"

#include "maker/make.h"

#include "encoding/format.h"

#include "impl.h"

mmerror_t motionmaskmaker_save(motionmaskmaker_t *maker,
                               char              *filename)
{
  FILE    *f;
  uint8_t  header[format_HEADER_SIZE];
  int      i;
  size_t   length;

  f = fopen(filename, "wb");
  if (f == NULL)
    return mmerror_FNF; // ideally could be a more appropriate error

  length = pack(header, "i2si",
                format_ID,
                maker->width, maker->height,
                maker->nframes);

  fwrite(header, 1, format_HEADER_SIZE, f);

  /* frames */

  for (i = 0; i < maker->nframes; i++)
  {
    uint8_t frame[format_FRAME_SIZE];

    length += pack(frame, "5si",
                   maker->width, maker->height,
                   0, 0,
                   0x3221, /* sources */
                   0x12212332 /* blend styles (NYI) */);

    fwrite(frame, 1, format_FRAME_SIZE, f);
  }

  /* offsets */

  for (i = 0; i < maker->noffsets; i++)
  {
    uint16_t o;

    o = maker->offsets[i] - maker->data;
    length += 2;
    fwrite(&o, 1, 2, f);
  }

  /* data */

  length += maker->ndata * sizeof(*maker->data);
  fwrite(maker->data, 1, maker->ndata * sizeof(*maker->data), f);

  fclose(f);

  logf_info("motionmaskmaker_save: motion mask saved, %d bytes long", length);

  return mmerror_OK;
}