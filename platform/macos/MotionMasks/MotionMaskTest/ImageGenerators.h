//
//  ImageGenerators.h
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_ImageGenerators_h
#define MotionMasks_ImageGenerators_h

#include "framebuf/pixelfmt.h"

CGImageRef createCGImageFromGradient(int                 width,
                                     int                 height,
                                     pixelfmt_rgbx8888_t start,
                                     pixelfmt_rgbx8888_t end,
                                     int                 direction);

#endif
