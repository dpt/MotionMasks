//
//  MMSinglePlayer.h
//  MotionMasks
//
//  MMPlayer singleton.
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_MMSinglePlayer_h
#define MotionMasks_MMSinglePlayer_h

#include "base/result.h"

#include "MMPlayer.h"

result_t MMSinglePlayer_instance(MMPlayer_t **player);
void MMSinglePlayer_destroy(void);

#endif
