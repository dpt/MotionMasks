//
//  ImageGenerators.c
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <stdint.h>
#include <stdlib.h>

#include <CoreGraphics/CoreGraphics.h>

#include "BitmapDrawing.h"
#include "BitmapUtils.h"

#include "framebuf/pixelfmt.h"

#include "ImageGenerators.h"

static void releaseDataCallback(void *info, const void *data, size_t size)
{
    (void) info;
    (void) size;
    
    free((void *) data);
}

CGImageRef createCGImageFromGradient(int                 width,
                                     int                 height,
                                     pixelfmt_rgbx8888_t start,
                                     pixelfmt_rgbx8888_t end,
                                     int                 direction)
{
  int                bpp;
  size_t             bytesPerRow;
  size_t             bytesPerImage;
  void              *base;
  void             (*drawGradient)(uint32_t *base,
                                   int       width,
                                   size_t    bytesPerRow,
                                   int       height);
  CGColorSpaceRef    rgbColourSpace;
  CGDataProviderRef  dataProvider;
  CGImageRef         image;
  
  bpp           = 32;
  bytesPerRow   = ROWBYTES16ALIGNED(width, bpp);
  bytesPerImage = bytesPerRow * height;
  
  base = malloc(bytesPerImage);
  if (base == NULL)
    return NULL;
  
  setupGradient32bpp(start, end);
  drawGradient = (direction == 0) ? drawXGradient32bpp : drawYGradient32bpp;  
  drawGradient(base, width, bytesPerRow, height);
    
  rgbColourSpace = CGColorSpaceCreateDeviceRGB();
  
  dataProvider = CGDataProviderCreateWithData(NULL,
                                              base,
                                              bytesPerImage,
                                              releaseDataCallback);
  
  image = CGImageCreate(width, height,
                        8, bpp,
                        bytesPerRow,
                        rgbColourSpace,
                        kCGImageAlphaNoneSkipLast,
                        dataProvider,
                        NULL, // decode array
                        false, // should interpolate
                        kCGRenderingIntentDefault);
    
  CGDataProviderRelease(dataProvider);
  
  CGColorSpaceRelease(rgbColourSpace);
  
  return image;
}
