//
//  ImageLoaders.h
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_ImageLoaders_h
#define MotionMasks_ImageLoaders_h

#include <CoreGraphics/CoreGraphics.h>

CGImageRef createCGImageFromJPEGFile(const char *filename);
CGImageRef createCGImageFromPNGFile(const char *filename);

#endif
