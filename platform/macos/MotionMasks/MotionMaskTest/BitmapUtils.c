//
//  BitmapUtils.c
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#include "framebuf/pixelfmt.h"

#include "BitmapUtils.h"

// -----------------------------------------------------------------------------

CFDataRef copyImagePixels(CGImageRef image)
{
  return CGDataProviderCopyData(CGImageGetDataProvider(image));
}

// -----------------------------------------------------------------------------

pixelfmt_t bitmapInfoToPixelfmt(CGBitmapInfo bitmapInfo)
{
  switch (bitmapInfo)
  {
  case kCGImageAlphaNone:
    return pixelfmt_y8;
  case kCGImageAlphaPremultipliedLast:  /* For example, premultiplied RGBA */
    return pixelfmt_rgba8888;
  case kCGImageAlphaPremultipliedFirst: /* For example, premultiplied ARGB */
    return pixelfmt_argb8888;
  case kCGImageAlphaLast:               /* For example, non-premultiplied RGBA */
    return pixelfmt_rgba8888;
  case kCGImageAlphaFirst:              /* For example, non-premultiplied ARGB */
    return pixelfmt_argb8888;
  case kCGImageAlphaNoneSkipLast:       /* For example, RBGX. */
    return pixelfmt_rgbx8888;
  case kCGImageAlphaNoneSkipFirst:      /* For example, XRGB. */
    return pixelfmt_xrgb8888;
  case kCGImageAlphaOnly:               /* No color data, alpha data only */
    break;
  default:
    break;
  }

  //NSLog(@"Unexpected CGBitmapInfo format: %d.", bitmapInfo);
  return pixelfmt_unknown;
}

// update this to match above, or factor out both
CGBitmapInfo PixelfmtTobitmapInfo(pixelfmt_t pixelfmt)
{
  switch (pixelfmt)
  {
  case pixelfmt_y8:
    return kCGImageAlphaNone;
  case pixelfmt_rgbx8888:
    return kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder32Big; // RGBX
  case pixelfmt_xrgb8888:
    return kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big; // XRGB
  default:
    break;
  }

  //NSLog(@"Unexpected pixelfmt_t format: %d.", pixelfmt);
  return 0;
}
