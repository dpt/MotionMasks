//
//  MMPlayer.c
//  MotionMasks
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <assert.h>
#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#include "framebuf/pixelfmt.h"
#include "framebuf/screen.h"

#include "player/play.h"

#include "BitmapUtils.h"
#include "BitmapTransform.h"
#include "ImageLoaders.h"
#include "ImageGenerators.h"
#include "Utils.h"

#include "MMSinglePlayer.h"

#include "MMPlayer.h"

// -----------------------------------------------------------------------------

// Define this to load images, as opposed to generating them.
#define LOAD_IMAGES

// Number of loops to use when plotting, for benchmarking.
#define BENCHMARK_LOOPS 1

// -----------------------------------------------------------------------------

#ifdef LOAD_IMAGES
#define MAX_SOURCE_IMAGES 20
#else
#define MAX_SOURCE_IMAGES 5
#endif

// -----------------------------------------------------------------------------

struct MMPlayer
{
  motionmaskplayer_t *motionMaskPlayer;
  int                 width, height; /* dimensions of motion mask */

  screen_t            screen;

  bitmap_t            sourceBitmaps[MAX_SOURCE_IMAGES];
  const bitmap_t     *sourceBitmapList[MAX_SOURCE_IMAGES];

  CGImageRef          sourceImages[MAX_SOURCE_IMAGES];
  CFDataRef           sourceData[MAX_SOURCE_IMAGES];

  char              **sourceImageFilenames;
  int                 nSourceImages;
  char               *sourceImageFilenamesBuffer;
};

// -----------------------------------------------------------------------------

result_t MMPlayer_create(MMPlayer_t **ptester)
{
  MMPlayer_t *tester;

  tester = calloc(1, sizeof(*tester));
  if (tester == NULL)
    return result_OOM;

  *ptester = tester;

  return result_OK;
}

void MMPlayer_destroy(MMPlayer_t *doomed)
{
  int i;

  if (doomed == NULL)
    return;

  free(doomed->screen.base);

  for (i = 0; i < doomed->nSourceImages; i++)
  {
    CGImageRelease(doomed->sourceImages[i]);
    doomed->sourceImages[i] = NULL;
  }

  free(doomed);
}

// -----------------------------------------------------------------------------

screen_t *MMPlayer_getScreen(MMPlayer_t *tester)
{
  return &tester->screen;
}

// -----------------------------------------------------------------------------

result_t MMPlayer_setup(MMPlayer_t *tester,
                        const char *filename,
                        int         width,
                        int         height,
                        const char *sourceDirs[],
                        int         nSourceDirs)
{
#ifndef LOAD_IMAGES
  static const struct
  {
    pixelfmt_rgbx8888_t start, end;
    int                 direction;
  }
  gradients[MAX_SOURCE_IMAGES] =
  {
    { 0x00000000, 0x000000FF, 1 },
    { 0x00000000, 0x0000FF00, 0 },
    { 0x00000000, 0x0000FFFF, 1 },
    { 0x00000000, 0x00FF0000, 0 },
    { 0x00000000, 0x00FF00FF, 1 },
  };
#endif

  result_t     err;
  MMPlayer_t   newt;
  uint8_t     *rawScreen = NULL;
  int          i;
  CGBitmapInfo bitmapInfo;
  pixelfmt_t   pixelfmt;
  int          rowbytes;

  newt = *tester;

  // load motion mask

  err = motionmaskplayer_create(&newt.motionMaskPlayer);
  if (err)
    goto failure;

  err = motionmaskplayer_load(newt.motionMaskPlayer, filename);
  if (err)
    goto failure;

  motionmaskplayer_get_dimensions(newt.motionMaskPlayer,
                                  &newt.width, &newt.height);

  // set up screen

  rowbytes = ROWBYTES16ALIGNED(width, 32);

  rawScreen = malloc(height * rowbytes);
  if (rawScreen == NULL)
  {
    err = result_OOM;
    goto failure;
  }

  newt.screen.width    = width;
  newt.screen.height   = height;
  newt.screen.format   = pixelfmt_rgbx8888;
  newt.screen.rowbytes = rowbytes;
  newt.screen.clip.x0  = 0;
  newt.screen.clip.y0  = 0;
  newt.screen.clip.x1  = newt.screen.width;
  newt.screen.clip.y1  = newt.screen.height;
  newt.screen.base     = rawScreen;

  // set up sources

#ifdef LOAD_IMAGES
  // scan for images
  err = findfilesbyregexp(sourceDirs,
                          nSourceDirs,
                          ".*\\.jpg",
                          &newt.sourceImageFilenames,
                          &newt.nSourceImages,
                          &newt.sourceImageFilenamesBuffer);
  if (err)
    goto failure;
#else
  newt.nSourceImages = MAX_SOURCE_IMAGES;
#endif

  // we could check that they're the same depth and dimensions but we can
  // ignore that and use this as a test for motionmask interface itself (it
  // ought to check)

  for (i = 0; i < newt.nSourceImages; i++)
  {
#ifdef LOAD_IMAGES
    newt.sourceImages[i] = createCGImageFromJPEGFile(newt.sourceImageFilenames[i]);
#else
    newt.sourceImages[i] = createCGImageFromGradient(320,
                                                     480, // HACK
                                                     gradients[i].start,
                                                     gradients[i].end,
                                                     gradients[i].direction);
#endif

    bitmapInfo = CGImageGetBitmapInfo(newt.sourceImages[i]);
    pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
    if (pixelfmt == pixelfmt_unknown)
    {
      err = result_BAD_ARG;
      goto failure;
    }

    newt.sourceData[i] = copyImagePixels(newt.sourceImages[i]);
    if (newt.sourceData[i] == NULL)
    {
      err = result_BAD_ARG;
      goto failure;
    }

    newt.sourceBitmaps[i].width    = (int) CGImageGetWidth(newt.sourceImages[i]);
    newt.sourceBitmaps[i].height   = (int) CGImageGetHeight(newt.sourceImages[i]);
    newt.sourceBitmaps[i].format   = pixelfmt;
    newt.sourceBitmaps[i].rowbytes = (int) CGImageGetBytesPerRow(newt.sourceImages[i]);
    newt.sourceBitmaps[i].base     = (void *) CFDataGetBytePtr(newt.sourceData[i]);
  }

  *tester = newt;

  /* fixup pointers */

  for (i = 0; i < newt.nSourceImages; i++)
    tester->sourceBitmapList[i] = &tester->sourceBitmaps[i];

  return result_OK;


failure:

  fprintf(stderr, "setupMotionMaskPlot: failure: err=%d\n", err);

  if (rawScreen) /* if we got as far as setting up 'newt' ... */
  {
    free(newt.screen.base);
  }

  return err;
}

result_t MMPlayer_render(MMPlayer_t *tester, int x, int y, box_t *dirty)
{
  static const pixelfmt_rgbx8888_t backgroundColour = 0x00808080;

  static int totalframes = 0;

  result_t err;
  int      i;
  int      maxframes = motionmaskplayer_get_frames(tester->motionMaskPlayer);

  assert(tester);
  assert(dirty);

  /* clear the screen */

  memset_pattern4(tester->screen.base,
                  &backgroundColour,
                  tester->screen.height * tester->screen.rowbytes);

  /* plot the motion mask */

  {
    static int frame         = 0;
    static int rotateSources = 0;

    if (rotateSources)
    {
      const bitmap_t *tmp;
      int             k;

      /* rotate the source bitmap list such that the final image shown is used as the next start image */

      tmp = tester->sourceBitmapList[0];

      /* could memmove instead of loop here */
      for (k = 0; k < tester->nSourceImages - 1; k++)
        tester->sourceBitmapList[k] = tester->sourceBitmapList[k + 1];

      tester->sourceBitmapList[k] = tmp;

      rotateSources = 0;
    }

    for (i = 0; i < BENCHMARK_LOOPS; i++) /* loop for benchmarking */
    {
      err = motionmaskplayer_plot(tester->motionMaskPlayer,
                                  tester->sourceBitmapList,
                                  tester->nSourceImages,
                                  &tester->screen,
                                  x, y,
                                  frame);
      if (err)
        return err;

      totalframes++;
    }

    if (++frame >= maxframes)
    {
      frame = 0;
      rotateSources = 1;
    }
  }
  
  if ((totalframes % 100) == 0)
    printf("%d frames drawn\n", totalframes);

  dirty->x0 = x;
  dirty->y0 = y;
  dirty->x1 = x + tester->width;
  dirty->y1 = y + tester->height;

  return result_OK;
}
