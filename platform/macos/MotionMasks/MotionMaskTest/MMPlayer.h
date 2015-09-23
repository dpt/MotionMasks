//
//  MMPlayer.h
//  MotionMasks
//
//  MotionMask + screen + image loading
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_MMPlayer_h
#define MotionMasks_MMPlayer_h

#include "base/result.h"

#include "framebuf/screen.h"

typedef struct MMPlayer MMPlayer_t;

result_t MMPlayer_create(MMPlayer_t **player);
void MMPlayer_destroy(MMPlayer_t *doomed);

screen_t *MMPlayer_getScreen(MMPlayer_t *player);

result_t MMPlayer_setup(MMPlayer_t *player,
                        const char *filename, /* of motion mask */
                        int         width,
                        int         height,
                        const char *sourceDirs[],
                        int         nSourceDirs);

result_t MMPlayer_render(MMPlayer_t *player, int x, int y, box_t *dirty);

#endif
