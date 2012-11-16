/* impl.h */

#ifndef MAKER_IMPL_H
#define MAKER_IMPL_H

#include "base/types.h"

#include "mm/types.h"

struct motionmaskmaker
{
  int32_t           width, height;

  mmframeindex_t   *frames;
  int32_t           nframes;

  mmoffset_t       *offsets;
  int32_t           noffsets;

  mmdata_t         *data;
  int32_t           ndata;
};

mmerror_t encode_row_y8(const void *vsrc,
                        int         nsrcpix,
                        uint8_t    *dst,
                        size_t      ndstbytes,
                        size_t     *dstused);

#endif /* MAKER_IMPL_H */
