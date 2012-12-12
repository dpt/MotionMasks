/* impl.h */

#ifndef PLAYER_IMPL_H
#define PLAYER_IMPL_H

#include "base/types.h"

#include "mm/types.h"

typedef unsigned int player_flags_t;

typedef struct frame
{
  int32_t           width, height;
  int32_t           x, y;
  uint16_t          source;     /* 0xSS - two 4-bit sources */
  int32_t           start;      /* index of start entry in 'offsets' */
}
frame_t;

#define player_FLAGS_LOADED (1u << 0)

struct motionmaskplayer
{
  player_flags_t    flags;

  int32_t           width, height;

  frame_t          *frames;
  int32_t           nframes;

  mmoffset_t       *offsets;

  mmdata_t         *data;
  size_t            ndata;
};

#endif /* PLAYER_IMPL_H */
