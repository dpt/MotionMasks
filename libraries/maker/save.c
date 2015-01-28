/* save.c */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "base/result.h"
#include "base/debug.h"
#include "base/types.h"

#include "utils/pack.h"

#include "maker/make.h"

#include "mm/format.h"

#include "impl.h"

result_t motionmaskmaker_save(motionmaskmaker_t *maker,
                              const char        *filename)
{
  FILE   *f;
  uint8_t header[format_HEADER_SIZE];
  int     i;
  size_t  length;

  f = fopen(filename, "wb");
  if (f == NULL)
    return result_FILE_NOT_FOUND; // ideally could be a more appropriate error

  length = pack(header, "i2si",
                format_ID,
                maker->width, maker->height,
                maker->nframes);

  fwrite(header, 1, format_HEADER_SIZE, f);

  /* frames */

  for (i = 0; i < maker->nframes; i++)
  {
    uint8_t frame[format_FRAME_SIZE];

    length += pack(frame, "4sc",
                   maker->width, maker->height,
                   0, 0,
                   0x10 /* sources (A = 0, B = 1) */);

    fwrite(frame, 1, format_FRAME_SIZE, f);
  }

  /* offsets */

  for (i = 0; i < maker->noffsets; i++)
  {
    ptrdiff_t diff;
    uint16_t  o;

    diff = maker->offsets[i] - maker->data;
    assert(diff <= UINT16_MAX);

    o = (uint16_t) diff; // ptrdiff_t
    length += 2;
    fwrite(&o, 1, 2, f);
  }

  /* data */

  length += maker->ndata * sizeof(*maker->data);
  fwrite(maker->data, 1, maker->ndata * sizeof(*maker->data), f);

  fclose(f);

  logf_info("motionmaskmaker_save: motion mask saved, %zd bytes long", length);

  return result_OK;
}
