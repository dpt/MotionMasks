/* xbgr8888.c */

#include <string.h>

#include "framebuf/pixelfmt.h"

#include "plot/span.h"

#include "plot/span-xbgr8888.h"

#include "all8888.h"
#include "xbgr8888.h"

#define RED_SHIFT   24
#define GREEN_SHIFT 16
#define BLUE_SHIFT  8
#define X_SHIFT     0

#define RED_MASK   (0xFFu << RED_SHIFT)
#define GREEN_MASK (0xFFu << GREEN_SHIFT)
#define BLUE_MASK  (0xFFu << BLUE_SHIFT)
#define X_MASK     (0xFFu << X_SHIFT)

// splats any alpha (but this is an 'X' format so we don't care)
// critical - optimise me once proved working
static pixelfmt_xbgr8888_t blend_pix_xbgr8888(pixelfmt_xbgr8888_t src1,
                                              pixelfmt_xbgr8888_t src2,
                                              int                 alpha)
{
  pixelfmt_xbgr8888_t r1, g1, b1;
  pixelfmt_xbgr8888_t r2, g2, b2;

  if (alpha == 0)
    return src1;
  if (alpha == 255)
    return src2;

  alpha += alpha >= 128;

  r1 = (src1 & RED_MASK) >> RED_SHIFT;
  r2 = (src2 & RED_MASK) >> RED_SHIFT;
  r1 = (r1 * (256 - alpha) + r2 * alpha) >> 8;

  g1 = (src1 & GREEN_MASK) >> GREEN_SHIFT;
  g2 = (src2 & GREEN_MASK) >> GREEN_SHIFT;
  g1 = (g1 * (256 - alpha) + g2 * alpha) >> 8;

  b1 = (src1 & BLUE_MASK) >> BLUE_SHIFT;
  b2 = (src2 & BLUE_MASK) >> BLUE_SHIFT;
  b1 = (b1 * (256 - alpha) + b2 * alpha) >> 8;

  return (r1 << RED_SHIFT) | (g1 << GREEN_SHIFT) | (b1 << BLUE_SHIFT) | X_MASK;
}

void span_xbgr8888_blendconst(void       *vdst,
                              const void *vsrc1,
                              const void *vsrc2,
                              int         length,
                              int         alpha)
{
  pixelfmt_xbgr8888_t       *dst  = vdst;
  const pixelfmt_xbgr8888_t *src1 = vsrc1;
  const pixelfmt_xbgr8888_t *src2 = vsrc2;

  while (length--)
  {
    *dst = blend_pix_xbgr8888(*src1, *src2, alpha);

    dst++;
    src1++;
    src2++;
  }
}

void span_xbgr8888_blendarray(void          *vdst,
                              const void    *vsrc1,
                              const void    *vsrc2,
                              int            length,
                              const alpha_t *alphas)
{
  pixelfmt_xbgr8888_t       *dst  = vdst;
  const pixelfmt_xbgr8888_t *src1 = vsrc1;
  const pixelfmt_xbgr8888_t *src2 = vsrc2;

  while (length--)
  {
    *dst = blend_pix_xbgr8888(*src1, *src2, *alphas);

    dst++;
    src1++;
    src2++;
    alphas++;
  }
}

const span_t span_xbgr8888 =
{
  pixelfmt_xbgr8888,
  span_all8888_copy,
  span_xbgr8888_blendconst,
  span_xbgr8888_blendarray,
};

