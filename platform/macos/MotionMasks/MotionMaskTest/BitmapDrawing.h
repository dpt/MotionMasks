//
//  BitmapDrawing.h
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_BitmapDrawing_h
#define MotionMasks_BitmapDrawing_h

#include <stdint.h>

// draw a checkerboard
void fillWithCheckerboard32bpp(uint8_t *base,
                          int      width,
                          size_t   bytesPerRow,
                          int      height,
                          uint32_t pixel);

// build temporary structure needed for gradient rendering
void setupGradient32bpp(uint32_t startColour,
                        uint32_t endColour);

// render a gradient horizontally
void drawXGradient32bpp(uint32_t *base,
                        int       width,
                        size_t    bytesPerRow,
                        int       height);

// render a gradient vertically
void drawYGradient32bpp(uint32_t *base,
                        int       width,
                        size_t    bytesPerRow,
                        int       height);

#endif
