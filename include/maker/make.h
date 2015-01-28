/* make.h -- motion mask encoder */

// animated mask library. animates by blending frames via a compact mask
// definition.

#ifndef MOTIONMASK_MAKER_H
#define MOTIONMASK_MAKER_H

#include "base/result.h"

#include "framebuf/bitmap-set.h"

typedef struct motionmaskmaker motionmaskmaker_t;

result_t motionmaskmaker_create(motionmaskmaker_t **maker);
void motionmaskmaker_destroy(motionmaskmaker_t *doomed);

void motionmaskmaker_set_config(motionmaskmaker_t *maker,
                                int                config_stuff);

result_t motionmaskmaker_pack(motionmaskmaker_t  *maker,
                              const bitmap_set_t *bitmaps);

// load and save via estream?

result_t motionmaskmaker_save(motionmaskmaker_t *maker,
                              const char        *filename);

#endif /* MOTIONMASK_MAKER_H */
