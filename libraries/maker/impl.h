/* impl.h */

#ifndef MAKER_IMPL_H
#define MAKER_IMPL_H

#include "base/types.h"

#include "mm/types.h"
#include "mm/codes.h"

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

void encode_start(encstate_t *state);

result_t encode_row_y8(encstate_t *state,
                          const void *vsrc,
                          int         nsrcpix,
                          uint8_t    *dst,
                          size_t      ndstbytes,
                          size_t     *dstused);

void encode_stop(encstate_t *state);

#endif /* MAKER_IMPL_H */
