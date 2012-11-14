/* impl.h */

#ifndef MAKER_IMPL_H
#define MAKER_IMPL_H

#include "base/types.h"

#include "mmtypes.h"

struct motionmaskmaker
{
  int32_t           width, height;

  frameindex_t     *frames;
  int32_t           nframes;

  offset_t         *offsets;
  int32_t           noffsets;

  data_t           *data;
  int32_t           ndata;
};

mmerror_t encode_row_y8(const void *vsrc,
                        int         nsrcpix,
                        uint8_t    *dst,
                        int         ndstbytes,
                        int        *dstused);

#endif /* MAKER_IMPL_H */
