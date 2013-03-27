//
//  MMaker.c
//  MotionMasks
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <assert.h>
#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#include "base/mmerror.h"
#include "framebuf/pixelfmt.h"
#include "framebuf/screen.h"

#include "maker/make.h"

#include "BitmapUtils.h"
#include "BitmapTransform.h"
#include "ImageLoaders.h"
#include "Utils.h"

#include "MMCommon.h"

#include "MMMaker.h"

// -----------------------------------------------------------------------------

static const char *makerSourceImageFilenames[] =
{
  PATH "output-0001.png",
  PATH "output-0002.png",
  PATH "output-0003.png",
  PATH "output-0004.png",
  PATH "output-0005.png",
  PATH "output-0006.png",
  PATH "output-0007.png",
  PATH "output-0008.png",
  PATH "output-0009.png",
  PATH "output-0010.png",
  PATH "output-0011.png",
  PATH "output-0012.png",
  PATH "output-0013.png",
  PATH "output-0014.png",
  PATH "output-0015.png",
  PATH "output-0016.png",
  PATH "output-0017.png",
  PATH "output-0018.png",
  PATH "output-0019.png",
  PATH "output-0020.png",
};

static const int nMakerSourceImageFilenames = NELEMS(makerSourceImageFilenames);

// -----------------------------------------------------------------------------

mmerror_t MMMaker_make(const char *filename)
{
  mmerror_t          mmerr;
  int                i;
  CGImageRef         makerSource[nMakerSourceImageFilenames];
  CGBitmapInfo       bitmapInfo;
  pixelfmt_t         pixelfmt;
  bitmap_set_t       makerBitmaps;
  CFDataRef          pixels[nMakerSourceImageFilenames];
  void              *makerBitmapBases[nMakerSourceImageFilenames];
  motionmaskmaker_t *maker;

  if (nMakerSourceImageFilenames <= 0)
    return mmerror_BAD_ARG;
  
  for (i = 0; i < nMakerSourceImageFilenames; i++)
    pixels[i] = NULL;

  mmerr = motionmaskmaker_create(&maker);
  if (mmerr)
    goto failure;
  
  for (i = 0; i < nMakerSourceImageFilenames; i++)
  {
    printf("loading %s", makerSourceImageFilenames[i]);

    makerSource[i] = createCGImageFromPNGFile(makerSourceImageFilenames[i]);

    bitmapInfo = CGImageGetBitmapInfo(makerSource[i]);
    pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
    if (pixelfmt == pixelfmt_unknown)
    {
      printf("MMMaker_make: Unknown pixelfmt.");
      return mmerror_BAD_ARG;
    }

    // bodge pixelfmt to be something we can currently cope with

    if (pixelfmt == pixelfmt_rgba8888)
      pixelfmt = pixelfmt_rgbx8888;
    if (pixelfmt == pixelfmt_abgr8888)
      pixelfmt = pixelfmt_xbgr8888;

    // turn the image into greyscale if it's anything else

    if (pixelfmt != pixelfmt_y8)
    {
      CGImageRef greyCopy;

      greyCopy = BitmapTransform_createGreyscaleCopy(makerSource[i]);

      CGImageRelease(makerSource[i]);

      makerSource[i] = greyCopy;

      bitmapInfo = CGImageGetBitmapInfo(makerSource[i]);
      pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
      if (pixelfmt == pixelfmt_unknown)
        return mmerror_BAD_ARG;
    }

    pixels[i] = copyImagePixels(makerSource[i]);
    if (pixels[i] == NULL)
      goto failure;

    makerBitmapBases[i] = (void *) CFDataGetBytePtr(pixels[i]);
  }

  makerBitmaps.width    = (int) CGImageGetWidth(makerSource[0]);
  makerBitmaps.height   = (int) CGImageGetHeight(makerSource[0]);
  makerBitmaps.format   = pixelfmt;
  makerBitmaps.rowbytes = (int) CGImageGetBytesPerRow(makerSource[0]);
  makerBitmaps.nbases   = nMakerSourceImageFilenames;
  makerBitmaps.bases    = makerBitmapBases;

  mmerr = motionmaskmaker_pack(maker, &makerBitmaps);
  if (mmerr)
    goto failure;

  mmerr = motionmaskmaker_save(maker, filename);
  if (mmerr)
    goto failure;

  /* cleanup */

failure:

  for (i = 0; i < nMakerSourceImageFilenames; i++)
    if (pixels[i])
      CFRelease(pixels[i]);

  motionmaskmaker_destroy(maker);
  maker = NULL;

  return mmerr;
}
