//
//  MMPlayer.h
//  MotionMasks
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_MMPlayer_h
#define MotionMasks_MMPlayer_h

#include "base/mmerror.h"

#include "framebuf/screen.h"

typedef struct MMPlayer MMPlayer_t;

mmerror_t MMPlayer_create(MMPlayer_t **player);
void MMPlayer_destroy(MMPlayer_t *doomed);

// FIXME: Exposing the screen_t maybe bad form.
screen_t *MMPlayer_getScreen(MMPlayer_t *player);

mmerror_t MMPlayer_setup(MMPlayer_t *player,
                         const char *filename,
                         int         width,
                         int         height);

mmerror_t MMPlayer_render(MMPlayer_t *player, int x, int y);

#endif
