/* --------------------------------------------------------------------------
 *    Name: stream.h
 * Purpose: Stream interface
 * ----------------------------------------------------------------------- */

/**
 * \file Stream (interface).
 *
 * Data source.
 */

#ifndef STREAM_H
#define STREAM_H

#include "base/mmerror.h"

#define mmerror_STREAM_BAD_SEEK   (mmerror_BASE_STREAM + 0)
#define mmerror_STREAM_CANT_SEEK  (mmerror_BASE_STREAM + 1)
#define mmerror_STREAM_UNKNOWN_OP (mmerror_BASE_STREAM + 2)

typedef enum stream_opcode
{
  stream_IN_MEMORY /* query whether stream is contained in memory,
                    * returns an int */
}
stream_opcode_t;

/** A type which holds a size or offset within a stream. */
typedef size_t stream_size_t; // ideally make this 64-bit where supported

typedef struct stream stream_t;

typedef mmerror_t stream_op_t(stream_t        *s,
                              stream_opcode_t  opcode,
                              void            *opaque);
typedef mmerror_t stream_seek_t(stream_t *s, stream_size_t pos);
typedef int       stream_get_t(stream_t *s);
typedef stream_size_t stream_fill_t(stream_t *s, stream_size_t need);
typedef stream_size_t stream_length_t(stream_t *s);
typedef void      stream_destroy_t(stream_t *doomed);

struct stream
{
  const unsigned char *buf; /* current buffer pointer */
  const unsigned char *end; /* end of buffer pointer (exclusive - points to the char after buffer end) */

  mmerror_t            last; /* last error. set when we return EOF? */

  stream_op_t         *op;
  stream_seek_t       *seek;
  stream_get_t        *get;
  stream_fill_t       *fill;
  stream_length_t     *length;
  stream_destroy_t    *destroy;
};

/* Main entry points */
stream_op_t      stream_op;
stream_seek_t    stream_seek;
stream_length_t  stream_length;
stream_destroy_t stream_destroy;

/* Get a byte from a stream. */
#define stream_getc(s) (((s)->buf != (s)->end) ? *(s)->buf++ : (s)->get(s))

/* Put back the last byte gotten. */
#define stream_ungetc(s) --(s)->buf

/* Returns the number of bytes remaining in the current buffer. */
#define stream_remaining(s) ((stream_size_t) ((s)->end - (s)->buf))

/* Returns the number of bytes remaining in the current buffer.
 * Will attempt to fill the buffer if it's found to be empty. */
#define stream_remaining_and_fill(s) \
        (stream_remaining(s) != 0 ? stream_remaining(s) : (s)->fill(s, 1))

/* As above but attempts to make 'need' bytes available. */
#define stream_remaining_need_and_fill(s, need) \
        (stream_remaining(s) >= (need) ? stream_remaining(s) : (s)->fill(s, need))

#endif /* STREAM_H */
