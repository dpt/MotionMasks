/* mmjpeg.h */

#ifndef MMJPEG_H
#define MMJPEG_H

#include "framebuf/pixelfmt.h"

int mmjpeg_load(const char  *filename,
                int         *pwidth,
                int         *pheight,
                pixelfmt_t  *pformat,
                void       **ppixels);

#endif

