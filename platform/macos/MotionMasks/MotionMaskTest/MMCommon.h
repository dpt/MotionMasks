//
//  MMCommon.h
//  MotionMasks
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_MMCommon_h
#define MotionMasks_MMCommon_h

#include "base/result.h"

#include "MMPlayer.h"

// Path to source images.
#define PATH "/Users/dave/SyncProjects/github/MotionMasks/"

result_t MMCommon_Player_instance(MMPlayer_t **pplayer);
void MMCommon_Player_destroy(void);

#endif
