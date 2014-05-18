#include <stdio.h>
#include <stdlib.h>

#include <jpeglib.h>
#include <jerror.h>

#include "framebuf/pixelfmt.h"

#include "mmjpeg.h"

int mmjpeg_load(const char  *filename,
                int         *pwidth,
                int         *pheight,
                pixelfmt_t  *pformat,
                void       **ppixels)
{
  FILE                          *fd;
  struct jpeg_decompress_struct  cinfo;
  struct jpeg_error_mgr          jerr;
  JSAMPROW                       row_ptr[1];
  pixelfmt_t                     format;
  int                            bytespp;
  unsigned char                 *buffer;
  int                            rowbytes;
  int                            y;

  *pwidth  = 0;
  *pheight = 0;
  *pformat = pixelfmt_unknown;
  *ppixels = NULL;

  if ((fd = fopen(filename, "rb")) == NULL)
  {
    fprintf(stderr, "mmjpeg_load: can't open %s\n", filename);
    return 1;
  }

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, fd);

  (void) jpeg_read_header(&cinfo, TRUE);

  switch (cinfo.out_color_space)
  {
  case JCS_GRAYSCALE:
    format = pixelfmt_y8;
    bytespp = 1;
    break;

  case JCS_RGB:
    format = pixelfmt_bgrx8888; // framebuffer style
    bytespp = 4;
    break;

  default:
    fprintf(stderr, "mmjpeg_load: unsupported colour space\n");
    goto failure;
  }

  jpeg_start_decompress(&cinfo);

  rowbytes = cinfo.output_width * bytespp;

  buffer = malloc(rowbytes * cinfo.output_height);
  if (buffer == NULL)
    goto failure;

  row_ptr[0] = buffer;
  while (cinfo.output_scanline < cinfo.output_height)
  {
    (void) jpeg_read_scanlines(&cinfo, row_ptr, 1);
    row_ptr[0] += rowbytes;
  }

  jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  if (bytespp > 1)
  {
    int            w, h;
    unsigned char *row;

    /* convert RGB pixels */

    w = cinfo.output_width;
    h = cinfo.output_height;

    row = buffer;

    for (y = 0; y < h; y++)
    {
      unsigned char *p;
      unsigned int  *q;
      unsigned int  *start;
      unsigned int  *end;

      p =                  row + 3 * (w - 1);
      q = (unsigned int *)(row + 4 * (w - 1));

      start = (unsigned int *) row;

      for (; q >= start; q--)
      {
        *q = p[2] | (p[1] << 8) | (p[0] << 16);
        p -= 3;
      }

      row += rowbytes;
    }
  }

  *pwidth  = cinfo.image_width;
  *pheight = cinfo.image_height;
  *pformat = format;
  *ppixels = buffer;

  return 0;


failure:

  jpeg_destroy_decompress(&cinfo);

  return 1;
}

