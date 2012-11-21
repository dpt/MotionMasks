//
//  BitmapTransform.c
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include <CoreGraphics/CoreGraphics.h>

#include "BitmapUtils.h"

#include "BitmapTransform.h"

CGImageRef BitmapTransform_createGreyscaleCopy(CGImageRef sourceImage)
{
  size_t           width, height;
  int              bpp;
  size_t           bytesPerRow;
  size_t           bytesPerImage;
  void            *pixels;
  CGColorSpaceRef  greyColourSpace;
  CGContextRef     context;
  CGImageRef       greyscaleImage;
  
  width         = CGImageGetWidth(sourceImage);
  height        = CGImageGetHeight(sourceImage);
  bpp           = 8;
  bytesPerRow   = ROWBYTES16ALIGNED(width, bpp);
  bytesPerImage = bytesPerRow * height;
  
  pixels = malloc(bytesPerImage);
  if (pixels == NULL)
    return NULL;
  
  greyColourSpace = CGColorSpaceCreateDeviceGray();
  
  context = CGBitmapContextCreate(pixels,
                                  width,
                                  height,
                                  8, // bpc
                                  bytesPerRow,
                                  greyColourSpace,
                                  kCGBitmapByteOrderDefault | kCGImageAlphaNone);
  
  CGContextDrawImage(context, CGRectMake(0, 0, width, height), sourceImage);
  
  greyscaleImage = CGBitmapContextCreateImage(context);
  
  CGContextRelease(context);
  
  CGColorSpaceRelease(greyColourSpace);
  
  free(pixels);
  
  return greyscaleImage;
}
