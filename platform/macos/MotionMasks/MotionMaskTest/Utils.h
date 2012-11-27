//
//  Utils.h
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_Utils_h
#define MotionMasks_Utils_h

#include <stdint.h>

#define NELEMS(a) ((int)(sizeof(a) / sizeof((a)[0])))

void memset32(uint32_t *dest, uint32_t val, int len);

#endif
