/* mmerror.h -- error constants */

#ifndef BASE_MMERROR_H
#define BASE_MMERROR_H

typedef int mmerror_t;

#define mmerror_BASE_GENERIC      0x0000
#define mmerror_BASE_STREAM       0x0100
#define mmerror_BASE_PLAYER       0x0200

#define mmerror_OK                0
#define mmerror_OOM               1 /* out of memory */
#define mmerror_FNF               2 /* file not found */
#define mmerror_BAD_ARG           3
#define mmerror_BUFFER_OVERFLOW   4

#endif /* BASE_MMERROR_H */
