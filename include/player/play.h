/* play.h -- motion mask decoder */

#ifndef MOTIONMASK_PLAY_H
#define MOTIONMASK_PLAY_H

#include "base/mmerror.h"

#include "framebuf/bitmap.h"
#include "framebuf/screen.h"

#define mmerror_PLAYER_TRUNCATED_INPUT    (mmerror_BASE_PLAYER + 0)
#define mmerror_PLAYER_BAD_SIGNATURE      (mmerror_BASE_PLAYER + 1)
#define mmerror_PLAYER_INVALID_DIMENSIONS (mmerror_BASE_PLAYER + 2)
#define mmerror_PLAYER_INVALID_FRAME_DIMENSIONS (mmerror_BASE_PLAYER + 3)
#define mmerror_PLAYER_BAD_OFFSET         (mmerror_BASE_PLAYER + 4)

typedef struct motionmaskplayer motionmaskplayer_t;

mmerror_t motionmaskplayer_create(motionmaskplayer_t **player);
void motionmaskplayer_destroy(motionmaskplayer_t *doomed);

mmerror_t motionmaskplayer_load(motionmaskplayer_t *player,
                                const char         *filename);

/* maximum number of source images */
#define motionmaskplayer_MAXSOURCES 16

mmerror_t motionmaskplayer_plot(const motionmaskplayer_t *player,
                                const bitmap_t           *sources[],
                                int                       nsources,
                                const screen_t           *screen,
                                int                       x,
                                int                       y,
                                int                       frame);

/* Returns dimensions of motion mask. */
void motionmaskplayer_get_dimensions(const motionmaskplayer_t *player,
                                     int                      *width,
                                     int                      *height);

/* Returns number of frames in motion mask. */
int motionmaskplayer_get_frames(const motionmaskplayer_t *player);

#endif /* MOTIONMASK_PLAY_H */
