//
//  MMMaker.c
//  MotionMasks
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <sys/types.h>

#include <assert.h>
#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

/* DPTLib */

#include "base/debug.h"
#include "base/result.h"
#include "utils/array.h"

/* MotionMasks */

#include "framebuf/pixelfmt.h"
#include "framebuf/screen.h"

#include "maker/make.h"

#include "BitmapUtils.h"
#include "BitmapTransform.h"
#include "ImageLoaders.h"
#include "Utils.h"

#include "MMSinglePlayer.h"

#include "MMMaker.h"

// -----------------------------------------------------------------------------

result_t MMMaker_make(const char **sourceMaskDirs,
                      int          nSourceMaskDirs,
                      const char  *filename)
{
#define MAX_FILENAMES 500

  result_t           err;
  char             **sourceMaskFilenames;
  int                nSourceMaskFilenames = 0;
  char              *sourceMaskFilenamesBuffer = NULL;
  motionmaskmaker_t *maker = NULL;
  int                i;
  CGImageRef         makerSource[MAX_FILENAMES];
  CGBitmapInfo       bitmapInfo;
  pixelfmt_t         pixelfmt;
  bitmap_set_t       makerBitmaps;
  CFDataRef          pixels[MAX_FILENAMES];
  void              *makerBitmapBases[MAX_FILENAMES];

  err = findfilesbyregexp(sourceMaskDirs,
                          nSourceMaskDirs,
                          ".*\\.png",
                          &sourceMaskFilenames,
                          &nSourceMaskFilenames,
                          &sourceMaskFilenamesBuffer);
  if (err)
    goto failure;

  if (nSourceMaskFilenames <= 0 || nSourceMaskFilenames > MAX_FILENAMES)
  {
    err = result_TOO_BIG;
    goto failure;
  }

  for (i = 0; i < nSourceMaskFilenames; i++)
    pixels[i] = NULL;

  err = motionmaskmaker_create(&maker);
  if (err)
    goto failure;

  for (i = 0; i < nSourceMaskFilenames; i++)
  {
    logf_info("sourcing mask %d from %s", i, sourceMaskFilenames[i]);

    makerSource[i] = createCGImageFromPNGFile(sourceMaskFilenames[i]);

    bitmapInfo = CGImageGetBitmapInfo(makerSource[i]);
    pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
    if (pixelfmt == pixelfmt_unknown)
    {
      logf_error("MMMaker_make: Unknown pixelfmt.");
      return result_BAD_ARG;
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
        return result_BAD_ARG;
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
  makerBitmaps.nbases   = nSourceMaskFilenames;
  makerBitmaps.bases    = makerBitmapBases;

  err = motionmaskmaker_pack(maker, &makerBitmaps);
  if (err)
    goto failure;

  err = motionmaskmaker_save(maker, filename);
  if (err)
    goto failure;

  /* cleanup */

failure:

  for (i = 0; i < nSourceMaskFilenames; i++)
    if (pixels[i])
      CFRelease(pixels[i]);

  motionmaskmaker_destroy(maker);
  maker = NULL;

  free(sourceMaskFilenamesBuffer);
  free(sourceMaskFilenames);

  return err;
}
