/* play.h -- motion mask decoder */

#ifndef MOTIONMASK_PLAY_H
#define MOTIONMASK_PLAY_H

#include "base/result.h"

#include "framebuf/bitmap.h"
#include "framebuf/screen.h"

#define result_MMPLAYER_TRUNCATED_INPUT           (result_BASE_MMPLAYER + 0)
#define result_MMPLAYER_BAD_SIGNATURE             (result_BASE_MMPLAYER + 1)
#define result_MMPLAYER_INVALID_DIMENSIONS        (result_BASE_MMPLAYER + 2)
#define result_MMPLAYER_INVALID_FRAME_DIMENSIONS  (result_BASE_MMPLAYER + 3)
#define result_MMPLAYER_BAD_OFFSET                (result_BASE_MMPLAYER + 4)

typedef struct motionmaskplayer motionmaskplayer_t;

result_t motionmaskplayer_create(motionmaskplayer_t **player);
void motionmaskplayer_destroy(motionmaskplayer_t *doomed);

result_t motionmaskplayer_load(motionmaskplayer_t *player,
                               const char         *filename);

/* maximum number of source images */
#define motionmaskplayer_MAXSOURCES 16

result_t motionmaskplayer_plot(const motionmaskplayer_t *player,
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
