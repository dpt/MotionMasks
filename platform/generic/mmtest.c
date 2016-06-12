/* mmtest.c */

#ifdef __linux__
#define FRAMEBUFFER_TEST
#endif


#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/fb.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "player/play.h"

#include "mmjpeg.h"

#ifdef FRAMEBUFFER_TEST
const char *fb_name = "/dev/fb0";
void *m_FrameBuffer;
struct fb_fix_screeninfo m_FixInfo;
struct fb_var_screeninfo m_VarInfo;
int m_FBFD = -1;
#endif

static int get_ms(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void bench(const char *name, int *fb, int w, int h)
{
  const int frames = 3000;

  int  i;
  int *p;
  int  x, y;
  int  t = 0;
  int  start, end;

  printf("%s: starts\n", name);

  start = get_ms();

  for (i = 0; i < frames; i++)
  {
    p = (int *) fb;

    if (0)for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
          int r, g, b;

          r = (t + x * 255 / w) & 0xFF;
          g = (y * 255 / h - t) & 0xFF;
          b = rand() & 0xFF; //(t + (x ^ y)) & 0xFF;

          *p++ = (b << 0) | (g << 8) | (r << 16);
        }
    memset(p, (t >> 0) & 0xFF, w * h * 4);
    t++;
  }

  end = get_ms();

  printf("%s: %d frames in %dms = %.2ffps, %.2fMbytes/sec\n",
         name,
         frames,
         end - start,
         (double) frames / ((end - start) / 1000.0),
         ((double)(w * h * 4) / 1024.0) / ((end - start) / 1000.0));
}

int main(void)
{
  int rc;
  int screenbytes;
  void *fakescreen;
  int width, height;
  int rowbytes;
  void *base;

#ifdef FRAMEBUFFER_TEST
  /* Open the framebuffer device in read write */
  m_FBFD = open(fb_name, O_RDWR);
  if (m_FBFD < 0)
  {
    printf("Unable to open %s.\n", fb_name);
    goto failure;
  }

  /* Do Ioctl. Retrieve fixed screen info. */
  if (ioctl(m_FBFD, FBIOGET_FSCREENINFO, &m_FixInfo) < 0)
  {
    printf("get fixed screen info failed: %s\n", strerror(errno));
    goto failure;
  }

  /* Do Ioctl. Get the variable screen info. */
  if (ioctl(m_FBFD, FBIOGET_VSCREENINFO, &m_VarInfo) < 0)
  {
    printf("Unable to retrieve variable screen info: %s\n", strerror(errno));
    goto failure;
  }

  printf("red: %d %d %d\n", m_VarInfo.red.offset, m_VarInfo.red.length, m_VarInfo.red.msb_right);
  printf("green: %d %d %d\n", m_VarInfo.green.offset, m_VarInfo.green.length, m_VarInfo.green.msb_right);
  printf("blue: %d %d %d\n", m_VarInfo.blue.offset, m_VarInfo.blue.length, m_VarInfo.blue.msb_right);

  /* Calculate the size to mmap */
  screenbytes = m_FixInfo.line_length * m_VarInfo.yres;
  printf("Line length %d\n", m_FixInfo.line_length);

  /* Now mmap the framebuffer. */
  m_FrameBuffer = mmap(NULL, screenbytes, PROT_READ | PROT_WRITE, MAP_SHARED, m_FBFD, 0);
  if (m_FrameBuffer == NULL)
  {
    printf("mmap failed:\n");
    goto failure;
  }

  bench("real screen memory", m_FrameBuffer, m_VarInfo.xres, m_VarInfo.yres);

  width = m_VarInfo.xres;
  height = m_VarInfo.yres;
  rowbytes = m_FixInfo.line_length;
  base = m_FrameBuffer;
#else
  width = 640;
  height = 480;
  rowbytes = 640 * 4;
  screenbytes = rowbytes * height;
#endif

  fakescreen = malloc(screenbytes);
  if (fakescreen == NULL)
  {
    printf("malloc failed\n");
    goto failure;
  }

#ifndef FRAMEBUFFER_TEST
  base = fakescreen;
#endif

  bench("fake screen memory", fakescreen, width, height);

  if (1)
  {
    int         jwidth, jheight;
    pixelfmt_t  jformat;
    void       *jpixels;

    mmjpeg_load("../../a.jpg", &jwidth, &jheight, &jformat, &jpixels);

    /*{
    unsigned char *ps = pixels;
    unsigned char *pd = base;
    int            y;
    int            s_rowbytes = width * 4;
    int            d_rowbytes = rowbytes;

    for (y = 0; y < height; y++)
    {
    memcpy(pd, ps, width * 4);
    pd += d_rowbytes;
    ps += s_rowbytes;
    }
    }*/

    result_t            mmerr;
    motionmaskplayer_t *motionMaskPlayer;
    screen_t            screen;
    bitmap_t            sourceBitmaps[2];
    const bitmap_t     *sourceBitmapList[2];

    mmerr = motionmaskplayer_create(&motionMaskPlayer);
    if (mmerr)
      return 1;

    mmerr = motionmaskplayer_load(motionMaskPlayer, "test.momask");
    if (mmerr)
      return 1;

    screen.width    = width;
    screen.height   = height;
    screen.format   = pixelfmt_bgrx8888;
    screen.rowbytes = rowbytes;
    screen.clip.x0  = 0;
    screen.clip.y0  = 0;
    screen.clip.x1  = screen.width;
    screen.clip.y1  = screen.height;
    screen.base     = base;

    sourceBitmaps[0].width    = jwidth;
    sourceBitmaps[0].height   = jheight;
    sourceBitmaps[0].format   = jformat;
    sourceBitmaps[0].rowbytes = jwidth * 4;
    sourceBitmaps[0].base     = jpixels;

    // 2nd source is an inverted copy of the 1st
    sourceBitmaps[1].width    = jwidth;
    sourceBitmaps[1].height   = jheight;
    sourceBitmaps[1].format   = jformat;
    sourceBitmaps[1].rowbytes = -jwidth * 4;
    sourceBitmaps[1].base     = (unsigned char *) jpixels + (jheight - 1) * jwidth * 4;

    sourceBitmapList[0] = &sourceBitmaps[0];
    sourceBitmapList[1] = &sourceBitmaps[1];

    int nframes;
    int i;
    int x;
    int y;
    int iters;

    nframes = motionmaskplayer_get_frames(motionMaskPlayer);
    x = 0;
    y = 0;

    for (iters = 0; iters < 10000; iters++)
    {
      for (i = 0; i < nframes; i++)
      {
        int l;

        for (l = 0; l < 1; l++)
        {
          mmerr = motionmaskplayer_plot(motionMaskPlayer,
                                        sourceBitmapList,
                                        2,
                                        &screen,
                                        x, y,
                                        i);
          if (mmerr)
            return 1;
        }

        if (0)
        {
          struct timespec ts;
          ts.tv_sec  = 0;
          ts.tv_nsec = 100000000;
          nanosleep(&ts, NULL);
        }
      }

      fprintf(stderr, ".");

      if (++x >= screen.width)
      {
        x = 0;
        y++;
      }
    }

    motionmaskplayer_destroy(motionMaskPlayer);
    motionMaskPlayer = NULL;
  }

  rc = EXIT_SUCCESS;


cleanup:

#ifdef FRAMEBUFFER_TEST
  if (m_FBFD >= 0)
    close(m_FBFD);
#endif

  free(fakescreen);

  exit(rc);

failure:
  rc = EXIT_FAILURE;
  goto cleanup;
}

