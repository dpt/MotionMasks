//
//  MMMaker.h
//  MotionMasks
//
//  Created by David Thomas on 26/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_MMaker_h
#define MotionMasks_MMaker_h

#include "base/result.h"

// Builds a MotionMask from the specfied input files.
result_t MMMaker_make(const char **pngdirs,
                      int          npngdirs,
                      const char  *filename);

#endif
