/* load.c */

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// public headers
#include "base/mmerror.h"
#include "base/debug.h"
#include "base/types.h"
#include "utils/pack.h"
#include "io/stream.h"
#include "io/stream-stdio.h"
#include "utils/array.h"
#include "utils/minmax.h"

// my public header
#include "player/play.h"

// internal headers
#include "mm/codes.h"
#include "mm/format.h"
#include "impl.h"

static mmerror_t unpackfromstream(stream_t   *s,
                                  size_t      size,
                                  const char *format,
                                  ...)
{
  va_list args;
  
  if (stream_remaining_need_and_fill(s, size) < size)
    return mmerror_PLAYER_TRUNCATED_INPUT;

  va_start(args, format);
  
  s->buf += vunpack(s->buf, format, args);
  
  va_end(args);
  
  return mmerror_OK;
}

mmerror_t motionmaskplayer_load(motionmaskplayer_t *player,
                                const char         *filename)
{
  mmerror_t   err;
  FILE       *f = NULL;
  stream_t   *s = NULL;
  uint32_t    signature;
  frame_t    *frames  = NULL;
  int         start;
  int         i;
  int         totalheights;
  mmoffset_t *offsets = NULL;
  mmdata_t   *data    = NULL;
  int         dataused;
  int         dataallocated;

  if (player->flags & player_FLAGS_LOADED)
  {
    free(player->frames);
    free(player->offsets);
    free(player->data);

    player->flags &= ~player_FLAGS_LOADED;
  }

  f = fopen(filename, "rb");
  if (f == NULL)
  {
    err = mmerror_FNF;
    goto failure;
  }

  err = stream_stdio_create(f, 1024, &s);
  if (err)
    goto failure;

  f = NULL; /* 'f' is now owned by the stream code */

  /* read file header */
  
  err = unpackfromstream(s,
                         format_HEADER_SIZE,
                         "<i2hii",
                         &signature,
                         &player->width,
                         &player->height,
                         &player->nframes);
  if (err)
    goto failure;

  if (signature != format_ID)
  {
    debugf("header has bad signature");
    err = mmerror_PLAYER_BAD_SIGNATURE;
    goto failure;
  }

  debugf("header: width x height = %d x %d, nframes = %d",
         player->width,
         player->height,
         player->nframes);

  if (player->width <= 0 || player->height <= 0 || player->nframes <= 0)
  {
    debugf("header has invalid dimensions");
    err = mmerror_PLAYER_INVALID_DIMENSIONS;
    goto failure;
  }

  /* read frames */

  frames = malloc(player->nframes * sizeof(*frames));
  if (frames == NULL)
    goto oom;

  start = 0;

  for (i = 0; i < player->nframes; i++)
  {
    frame_t *frame = &frames[i];

    err = unpackfromstream(s,
                           format_FRAME_SIZE,
                           "<4hiSI",
                           &frame->width,
                           &frame->height,
                           &frame->x,
                           &frame->y,
                           &frame->source,
                           &frame->blendstyle);
    if (err)
      goto failure;

    debugf("frame %d: width x height = %d x %d, x,y = %d,%d, source = %x",
           i,
           frame->width,
           frame->height,
           frame->x,
           frame->y,
           frame->source);

    if (frame->width <= 0 || frame->height <= 0)
    {
      err = mmerror_PLAYER_INVALID_FRAME_DIMENSIONS;
      goto failure;
    }

    // FIXME: Expand checks for x,y and source fields.

    frame->start = start;

    start += player->height;
  }

  totalheights = start;

  /* unpack offsets. we'll need to work over them again later. */

  offsets = malloc(totalheights * sizeof(*offsets));
  if (offsets == NULL)
    goto oom;

  {
    int n;

    for (i = 0; i < totalheights; i += n)
    {
      stream_size_t remaining;

      remaining = stream_remaining_need_and_fill(s, 2);
      if (remaining == stream_EOF || remaining < 2)
      {
        err = mmerror_PLAYER_TRUNCATED_INPUT;
        goto failure;
      }

      /* read up to the limit of the size of data we're anticipating */
      n = MIN(totalheights - i, remaining / 2);
      if (n < 1)
      {
        err = mmerror_PLAYER_TRUNCATED_INPUT;
        goto failure;
      }

      // FIXME: Need an unpack() type to unpack into a pointer (i.e. 'I' or 'Q'
      // dependent on platform bit size).
      // s->buf += unpack(s->buf, "<*hI", n, offsets + i); // for 32-bit pointers
      s->buf += unpack(s->buf, "<*hQ", n, offsets + i); // for 64-bit pointers
    }
  }

  /* read data chunk */

  data          = NULL;
  dataused      = 0;
  dataallocated = 0;

  for (;;)
  {
    stream_size_t remaining;  // stream returns a ptrdiff_t actually

    remaining = stream_remaining_and_fill(s);
    if (remaining == stream_EOF)
      break; /* EOF (we assume) */

    if (array_grow((void **) &data,
                   sizeof(*data),
                   dataused,
                   &dataallocated,
                   (int) remaining,
                   1))
      goto oom;

    memcpy(data + dataused, s->buf, remaining);
    dataused += remaining;
    s->buf   += remaining;
  }

  if (data == NULL)
  {
    err = mmerror_PLAYER_TRUNCATED_INPUT;
    goto failure;
  }

  if (array_shrink((void **) &data,
                   sizeof(*data),
                   dataused,
                   &dataallocated))
    goto oom;

  /* fix up offsets table now we know where 'data' lives */

  for (i = 0; i < totalheights; i++)
  {
    mmoffset_t o;

    o = offsets[i];
    
    /* verify the offsets (as a truncated data chunk cannot be detected earlier
     * on) */
    if (o >= (mmoffset_t) dataused)
    {
      debugf("offset %d out of range\n", i);
      err = mmerror_PLAYER_BAD_OFFSET;
      goto failure;
    }
    
    offsets[i] = data + (ptrdiff_t) o;
  }

  stream_destroy(s);

  player->frames  = frames;
  player->offsets = offsets;
  player->data    = data;
  player->ndata   = dataused;

  player->flags |= player_FLAGS_LOADED;

  return mmerror_OK;


oom:

  err = mmerror_OOM;

failure:

  if (f)
    fclose(f);

  stream_destroy(s);

  free(frames);
  free(offsets);
  free(data);

  return err;
}

void motionmaskplayer_get_dimensions(const motionmaskplayer_t *player,
                                     int                      *width,
                                     int                      *height)
{
  assert(player);

  if (width)
    *width  = player->width;
  if (height)
    *height = player->height;
}

int motionmaskplayer_get_frames(const motionmaskplayer_t *player)
{
  assert(player);

  return player->nframes;
}
