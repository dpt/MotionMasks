//
//  Utils.c
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <stdint.h>

#include "Utils.h"

void memset32(uint32_t *dest, uint32_t val, int len)
{
  while (len--)
    *dest++ = val;
}
