//
//  BitmapUtils.c
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <CoreFoundation/CFData.h>
#include <CoreGraphics/CoreGraphics.h>

#include "BitmapUtils.h"

CFDataRef copyImagePixels(CGImageRef image)
{
  return CGDataProviderCopyData(CGImageGetDataProvider(image));
}
