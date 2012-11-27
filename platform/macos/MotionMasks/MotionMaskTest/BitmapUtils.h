//
//  BitmapUtils.h
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_BitmapUtils_h
#define MotionMasks_BitmapUtils_h

#include <CoreFoundation/CFData.h>
#include <CoreGraphics/CGImage.h>

#include "framebuf/pixelfmt.h"

// Calculate bytesPerRow rounded to a 16-byte boundary as Quartz prefers.
#define ROWBYTES16ALIGNED(width, bpp) ((((width) * (bpp) + 127) & ~127) >> 3)

// Return a copy of the pixel array of the specified CGImage.
CFDataRef copyImagePixels(CGImageRef image);

pixelfmt_t bitmapInfoToPixelfmt(CGBitmapInfo bitmapInfo);
CGBitmapInfo PixelfmtTobitmapInfo(pixelfmt_t pixelfmt);

#endif
