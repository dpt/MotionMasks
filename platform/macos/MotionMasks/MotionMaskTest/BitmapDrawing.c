//
//  BitmapDrawing.c
//  MotionMasks
//
//  Created by David Thomas on 18/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Utils.h"

#include "BitmapDrawing.h"

// -----------------------------------------------------------------------------

void fillWithCheckerboard32bpp(uint8_t *base,
                               int      width,
                               size_t   bytesPerRow,
                               int      height,
                               uint32_t pixel)
{
  unsigned int *bufi;
  int           x,y;

  bufi = (unsigned int *) base;
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++)
      bufi[(y * bytesPerRow) / 4 + x] = (x & 1) == (y & 1) ? 0x00000000 : pixel;
}

// -----------------------------------------------------------------------------

static uint32_t gradientTable32bpp[256];

void setupGradient32bpp(uint32_t startColour, uint32_t endColour)
{
  uint32_t *p;
  int       i;

  p = gradientTable32bpp;
  for (i = 0; i < 256; i++)
    *p++ = startColour + i * (endColour - startColour) / (256 - 1);
}

void drawXGradient32bpp(uint32_t *base,
                        int       width,
                        size_t    bytesPerRow,
                        int       height)
{
  uint32_t *p;
  int       x;
  int       y;

  p = base;
  for (x = 0; x < width; x++)
    *p++ = gradientTable32bpp[x * 255 / (width - 1)];

  p = base + (bytesPerRow >> 2);
  for (y = 1; y < height; y++)
  {
    memcpy(p, base, bytesPerRow);
    p += bytesPerRow >> 2;
  }
}

void drawYGradient32bpp(uint32_t *base,
                        int       width,
                        size_t    bytesPerRow,
                        int       height)
{
  int y;

  for (y = 0; y < height; y++)
  {
    memset32(base, gradientTable32bpp[y * 256 / height], width);
    base += bytesPerRow >> 2;
  }
}
