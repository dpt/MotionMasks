/* --------------------------------------------------------------------------
 *    Name: stream-stdio.c
 * Purpose: C standard IO stream implementation
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/error.h"

#include "io/stream.h"

#include "io/stream-stdio.h"

typedef struct stream_file
{
  stream_t       base;
  FILE          *file;
  stream_size_t  length;

  stream_size_t  bufsz;
  unsigned char  buffer[1]; /* flexible final member */
}
stream_file_t;

static mmerror_t stream_stdio_seek(stream_t *s, stream_size_t pos)
{
  stream_file_t *sf = (stream_file_t *) s;

  fseek(sf->file, pos, SEEK_SET);

  sf->base.buf = sf->base.end; /* force a re-fill */

  return mmerror_OK;
}

/* ensure we have at least one byte in the buffer. return it, or EOF. */
static int stream_stdio_get(stream_t *s)
{
  stream_file_t *sf = (stream_file_t *) s;
  size_t         remaining = stream_remaining(s);

  if (remaining == 0)
  {
    size_t read;

    read = fread(sf->buffer, 1, sf->bufsz, sf->file);
    if (read == 0 && feof(sf->file))
      return EOF;

    sf->base.buf = sf->buffer;
    sf->base.end = sf->buffer + read;
  }

  return *sf->base.buf++;
}

/* ensure we have at least 'need' bytes in the buffer. return 0, or EOF. */
static stream_size_t stream_stdio_fill(stream_t *s, stream_size_t need)
{
  stream_file_t *sf        = (stream_file_t *) s;
  size_t         remaining = stream_remaining(s);
  size_t         read;

  if (remaining >= need)
    return remaining; /* have enough bytes already */

  /* shift any remaining bytes to the start of the buffer */
  if (remaining)
    memmove(sf->buffer, sf->base.buf, remaining);

  /* try to fill the buffer */
  read = fread(sf->buffer + remaining, 1, sf->bufsz - remaining, sf->file);
  if (read == 0 && feof(sf->file))
    return EOF; /* encountered EOF but bytes may remain! */

  sf->base.buf = sf->buffer;
  sf->base.end = sf->buffer + remaining + read;

  return stream_remaining(s); /* success */
}

static stream_size_t stream_stdio_length(stream_t *s)
{
  stream_file_t *sf = (stream_file_t *) s;

  if (sf->length < 0)
  {
    long int pos;

    /* cache the file's length */

    pos = ftell(sf->file);
    fseek(sf->file, 0, SEEK_END);
    sf->length = ftell(sf->file);
    fseek(sf->file, pos, SEEK_SET);
  }

  return (int) sf->length;
}

static void stream_stdio_destroy(stream_t *doomed)
{
  stream_file_t *sf = (stream_file_t *) doomed;

  fclose(sf->file);
}

mmerror_t stream_stdio_create(FILE *f, int bufsz, stream_t **s)
{
  stream_file_t *sf;

  if (bufsz <= 0)
    bufsz = 128;

  assert(f);

  sf = malloc(offsetof(stream_file_t, buffer) + bufsz);
  if (!sf)
    return mmerror_OOM;

  sf->base.buf     =
    sf->base.end     = sf->buffer; /* force a fill on first use */

  sf->base.last    = mmerror_OK;

  sf->base.op      = NULL;
  sf->base.seek    = stream_stdio_seek;
  sf->base.get     = stream_stdio_get;
  sf->base.fill    = stream_stdio_fill;
  sf->base.length  = stream_stdio_length;
  sf->base.destroy = stream_stdio_destroy;

  sf->file   = f;
  sf->length = -1;
  sf->bufsz  = bufsz;

  *s = &sf->base;

  return mmerror_OK;
}
