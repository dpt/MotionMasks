//
//  ImageLoaders.c
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <CoreGraphics/CoreGraphics.h>

#include "ImageLoaders.h"

CGImageRef createCGImageFromJPEGFile(const char *filename)
{
  CGDataProviderRef dataProvider;
  CGImageRef        image;

  dataProvider = CGDataProviderCreateWithFilename(filename);

  image = CGImageCreateWithJPEGDataProvider(dataProvider,
                                            NULL,
                                            false,
                                            kCGRenderingIntentDefault);

  CGDataProviderRelease(dataProvider);

  return image;
}

CGImageRef createCGImageFromPNGFile(const char *filename)
{
  CGDataProviderRef dataProvider;
  CGImageRef        image;

  dataProvider = CGDataProviderCreateWithFilename(filename);

  image = CGImageCreateWithPNGDataProvider(dataProvider,
                                           NULL,
                                           false,
                                           kCGRenderingIntentDefault);

  CGDataProviderRelease(dataProvider);

  return image;
}

