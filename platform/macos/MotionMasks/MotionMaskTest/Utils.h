//
//  Utils.h
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#ifndef MotionMasks_Utils_h
#define MotionMasks_Utils_h

#include <stdint.h>

#include "base/result.h"

#define NELEMS(a) ((int)(sizeof(a) / sizeof((a)[0])))

void memset32(uint32_t *dest, uint32_t val, int len);

/**
 * Search through a set of directories and return a list of files whose names
 * match the specified regexp.
 *
 * This returns a list of filenames and a buffer block both of which should
 * be free()'d to dispose of.
 *
 * Similar to glob().
 *
 * \param[in]  dirs       Array of directories to search.
 * \param[in]  ndirs      Number of directories given.
 * \param[in]  pattern    Extended regexp pattern to match.
 * \param[out] filenames  Returned filenames.
 * \param[out] nfilenames Number of filenames returned.
 * \param[out] buffer     Buffer holding the filenames.
 *
 * \returns Result code.
 */
result_t findfilesbyregexp(const char **dirs,
                           int          ndirs,
                           const char  *pattern,
                           char      ***filenames,
                           int         *nfilenames,
                           char       **buffer);

#endif
