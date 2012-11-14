/* debug.h -- debug and log macros */

#ifndef BASE_DEBUG_H
#define BASE_DEBUG_H

#include <stdio.h>

/* Log a message - available in all builds. */
#define logf_info(fmt, ...)    fprintf(stderr, "(info) %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define logf_warning(fmt, ...) fprintf(stderr, "(warning) %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define logf_error(fmt, ...)   fprintf(stderr, "(ERROR) %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define logf_fatal(fmt, ...)   fprintf(stderr, "(FATAL) %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/* Log a debug message - available only in debug builds. */
#ifndef NDEBUG
#  define debugf(fmt, ...)     fprintf(stderr, "(debug) %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#  define check(err)           do { if (err) { logf_error("%x", err); goto failure; } while (0)
#  define sentinel             do { logf_error("sentinel"); goto failure; } while (0)
#else
#  define debugf(fmt, ...)
#  define check(err)
#  define sentinel
#endif

#endif /* BASE_DEBUG_H */
