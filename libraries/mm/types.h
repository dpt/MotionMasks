/* types.h -- Motion Mask types */

#ifndef MM_TYPES_H
#define MM_TYPES_H

#include "base/types.h"

/** Type of alpha values. */
typedef uint8_t mmalpha_t;

/** Type of motion mask data. */
typedef uint8_t mmdata_t;

/** Type of data pointer. */
typedef mmdata_t *mmoffset_t;

/** Type of frame pointer. */
typedef int32_t mmframeindex_t;

#endif /* MM_TYPES_H */
