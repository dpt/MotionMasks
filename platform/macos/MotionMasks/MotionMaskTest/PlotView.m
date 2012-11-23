// sloppy slapdash slipshot test app for momasks

#import <Foundation/NSTimer.h>

#import "PlotView.h"

#import "BitmapDrawing.h"
#import "BitmapTransform.h"
#import "BitmapUtils.h"
#import "ImageLoaders.h"
#import "ImageGenerators.h"
#import "Utils.h"

#import "player/play.h"
#import "maker/make.h"

// TODO
// slow update speed - maybe using OpenGL would be a better idea than Cocoa (more noticable when window is large)
// free objects (leaking stuff presently)

// DONE
// images getting drawn on non-whole pixel boundaries [fixed - round mouse coords down to integers]
// mystery blue left hand column [it's gone]
// image not updating - caching first run and never refreshing [fixed - recreate objects on each draw op :-( ]

// -----------------------------------------------------------------------------

// Path to source images.
#define PATH "/Users/dave/Dropbox/Projects/github/MotionMasks/"

// Define this to load images, as opposed to generating them.
//#define LOAD_IMAGES

// -----------------------------------------------------------------------------

static const int imageWidth = 320, imageHeight = 480;

static const int screenX = 32, screenY = 32; // where to plot
static const int screenWidth = imageWidth * 2, screenHeight = imageHeight;
static const int screenBPC = 8;
static const int screenBPP = 4 * screenBPC; // bits per pixel
static const size_t screenBytesPerRow   = ROWBYTES16ALIGNED(screenWidth, screenBPP);
static const size_t screenBytesPerImage = screenBytesPerRow * screenHeight;

static const char motionMaskFilename[] = "tmp.momask";

#ifdef LOAD_IMAGES

static const char *sourceImageFilenames[] =
{
  PATH "a.jpg",
  PATH "b.jpg",
  PATH "c.jpg",
  PATH "d.jpg",
  PATH "e.jpg",
};

#endif

static const int nSourceImages = 5; // NELEMS(sourceImageFilenames);

// -----------------------------------------------------------------------------

static const char *makerSourceImageFilenames[] =
{
  //    PATH "mm1.png",
  //    PATH "mm2.png",
  //    PATH "mm3.png"
  PATH "output-0001.png",
  PATH "output-0002.png",
  PATH "output-0003.png",
  PATH "output-0004.png",
  PATH "output-0005.png",
  PATH "output-0006.png",
  PATH "output-0007.png",
  PATH "output-0008.png",
  PATH "output-0009.png",
  PATH "output-0010.png",
  PATH "output-0011.png",
  PATH "output-0012.png",
  PATH "output-0013.png",
  PATH "output-0014.png",
  PATH "output-0015.png",
  PATH "output-0016.png",
  PATH "output-0017.png",
  PATH "output-0018.png",
  PATH "output-0019.png",
  PATH "output-0020.png",
};
static const int nMakerSourceImageFilenames = 20; // NELEMS(makerSourceImageFilenames);

// -----------------------------------------------------------------------------

static motionmaskplayer_t *motionMaskPlayer;

static bitmap_t            sourceBitmaps[nSourceImages];
static const bitmap_t     *sourceBitmapList[nSourceImages];

static screen_t            screen;

static CGImageRef          sourceImages[nSourceImages];
static CFDataRef           sourceData[nSourceImages];

static CGImageRef          screenImage;
static CGDataProviderRef   screenDataProvider;

static CGColorSpaceRef     colourSpace;

static int                 x,y;

// -----------------------------------------------------------------------------

@implementation PlotView

// -----------------------------------------------------------------------------

static pixelfmt_t bitmapInfoToPixelfmt(CGBitmapInfo bitmapInfo)
{
  switch (bitmapInfo)
  {
    case kCGImageAlphaNone:
      return pixelfmt_y8;
    case kCGImageAlphaPremultipliedLast:  /* For example, premultiplied RGBA */
      return pixelfmt_rgba8888;
    case kCGImageAlphaPremultipliedFirst: /* For example, premultiplied ARGB */
      return pixelfmt_argb8888;
    case kCGImageAlphaLast:               /* For example, non-premultiplied RGBA */
      return pixelfmt_rgba8888;
    case kCGImageAlphaFirst:              /* For example, non-premultiplied ARGB */
      return pixelfmt_argb8888;
    case kCGImageAlphaNoneSkipLast:       /* For example, RBGX. */
      return pixelfmt_rgbx8888;
    case kCGImageAlphaNoneSkipFirst:      /* For example, XRGB. */
      return pixelfmt_xrgb8888;
    case kCGImageAlphaOnly:               /* No color data, alpha data only */
      break;
    default:
      break;
  }
  
  NSLog(@"Unexpected CGBitmapInfo format: %d.", bitmapInfo);
  return pixelfmt_unknown;
}

// update this to match above, or factor out both
static CGBitmapInfo PixelfmtTobitmapInfo(pixelfmt_t pixelfmt)
{
  switch (pixelfmt)
  {
    case pixelfmt_y8:
      return kCGImageAlphaNone;
    case pixelfmt_rgbx8888:
      return kCGImageAlphaNoneSkipLast; // RGBX
    case pixelfmt_xrgb8888:
      return kCGImageAlphaNoneSkipFirst; // XRGB
    default:
      break;
  }
  
  NSLog(@"Unexpected pixelfmt_t format: %d.", pixelfmt);
  return 0;
}

// -----------------------------------------------------------------------------

-(mmerror_t)testMotionMaskCreate:(const char *)filename
{
  mmerror_t          mmerr = mmerror_OK;
  int                i;
  CGImageRef         makerSource[nMakerSourceImageFilenames];
  CGBitmapInfo       bitmapInfo;
  pixelfmt_t         pixelfmt;
  bitmap_set_t       makerBitmaps;
  CFDataRef          pixels[nMakerSourceImageFilenames];
  void              *makerBitmapBases[nMakerSourceImageFilenames];
  motionmaskmaker_t *maker;
  
  mmerr = motionmaskmaker_create(&maker);
  if (mmerr)
    goto failure;
  
  for (i = 0; i < nMakerSourceImageFilenames; i++)
  {
    NSLog(@"loading %s", makerSourceImageFilenames[i]);
    
    makerSource[i] = createCGImageFromPNGFile(makerSourceImageFilenames[i]);
    
    bitmapInfo = CGImageGetBitmapInfo(makerSource[i]);
    pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
    if (pixelfmt == pixelfmt_unknown)
    {
      NSLog(@"testMotionMaskCreate: Unknown pixelfmt.");
      return mmerror_BAD_ARG;
    }
    
    // bodge pixelfmt to be something we can currently cope with
    
    if (pixelfmt == pixelfmt_rgba8888)
      pixelfmt = pixelfmt_rgbx8888;
    if (pixelfmt == pixelfmt_abgr8888)
      pixelfmt = pixelfmt_xbgr8888;
    
    // turn the image into greyscale if it's anything else
    
    if (pixelfmt != pixelfmt_y8)
    {
      CGImageRef greyCopy;
      
      greyCopy = BitmapTransform_createGreyscaleCopy(makerSource[i]);
      
      CGImageRelease(makerSource[i]);
      
      makerSource[i] = greyCopy;
      
      bitmapInfo = CGImageGetBitmapInfo(makerSource[i]);
      pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
      if (pixelfmt == pixelfmt_unknown)
        return mmerror_BAD_ARG;
    }
    
    pixels[i] = copyImagePixels(makerSource[i]);
    assert(pixels[i]);
    
    makerBitmapBases[i] = (void *) CFDataGetBytePtr(pixels[i]);
  }
  
  makerBitmaps.width    = (int) CGImageGetWidth(makerSource[0]);
  makerBitmaps.height   = (int) CGImageGetHeight(makerSource[0]);
  makerBitmaps.format   = pixelfmt;
  makerBitmaps.rowbytes = (int) CGImageGetBytesPerRow(makerSource[0]);
  makerBitmaps.nbases   = nMakerSourceImageFilenames;
  makerBitmaps.bases    = makerBitmapBases;
  
  mmerr = motionmaskmaker_pack(maker, &makerBitmaps);
  if (mmerr)
    goto failure;
  
  mmerr = motionmaskmaker_save(maker, filename);
  if (mmerr)
    goto failure;
  
  mmerr = mmerror_OK;
  
failure:
  
  for (i = 0; i < nMakerSourceImageFilenames; i++)
    if (pixels[i])
      CFRelease(pixels[i]);
  
  motionmaskmaker_destroy(maker);
  maker = NULL;
  
  return mmerr;
}

-(mmerror_t)setupMotionMaskPlot:(const char *)filename
{
#ifndef LOAD_IMAGES
  static const struct
  {
    pixelfmt_rgbx8888_t start, end;
    int                 direction;
  }
  gradients[nSourceImages] =
  {
    { 0x00000000, 0x000000FF, 1 },
    { 0x00000000, 0x0000FF00, 0 },
    { 0x00000000, 0x0000FFFF, 1 },
    { 0x00000000, 0x00FF0000, 0 },
    { 0x00000000, 0x00FF00FF, 1 },
    // { 0x00000000, 0x00FFFF00, 0 },
  };
#endif
  
  mmerror_t    mmerr;
  int          i;
  CGBitmapInfo bitmapInfo;
  pixelfmt_t   pixelfmt;
  
  // load mm
  
  mmerr = motionmaskplayer_create(&motionMaskPlayer);
  if (mmerr)
    goto failure;
  
  mmerr = motionmaskplayer_load(motionMaskPlayer, filename);
  if (mmerr)
    goto failure;
  
  // set up sources
  
  // we could check that they're the same depth and dimensions but we can
  // ignore that and use this as a test for motionmask interface itself (it
  // ought to check)
  
  for (i = 0; i < nSourceImages; i++)
  {
#ifdef LOAD_IMAGES
    sourceImages[i] = createCGImageFromJPEGFile(sourceImageFilenames[i]);
#else
    sourceImages[i] = createCGImageFromGradient(imageWidth,
                                                imageHeight,
                                                gradients[i].start,
                                                gradients[i].end,
                                                gradients[i].direction);
#endif
    
    bitmapInfo = CGImageGetBitmapInfo(sourceImages[i]);
    pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
    if (pixelfmt == pixelfmt_unknown)
    {
      mmerr = mmerror_BAD_ARG;
      goto failure;
    }
    
    sourceData[i] = copyImagePixels(sourceImages[i]);
    if (sourceData[i] == NULL)
    {
      mmerr = mmerror_BAD_ARG;
      goto failure;
    }
    
    sourceBitmaps[i].width    = (int) CGImageGetWidth(sourceImages[i]);
    sourceBitmaps[i].height   = (int) CGImageGetHeight(sourceImages[i]);
    sourceBitmaps[i].format   = pixelfmt;
    sourceBitmaps[i].rowbytes = (int) CGImageGetBytesPerRow(sourceImages[i]);
    sourceBitmaps[i].base     = (void *) CFDataGetBytePtr(sourceData[i]);
    
    sourceBitmapList[i] = &sourceBitmaps[i];
  }
  
  // set up screen
  
  {
    uint8_t *rawScreen;
    
    rawScreen = malloc(screenBytesPerImage);
    if (rawScreen == NULL)
    {
      mmerr = mmerror_OOM;
      goto failure;
    }
    
    screen.width    = screenWidth;
    screen.height   = screenHeight;
    screen.format   = pixelfmt;
    screen.rowbytes = screenBytesPerRow;
    screen.clip.x0  = 0;
    screen.clip.y0  = 0;
    screen.clip.x1  = screen.width;
    screen.clip.y1  = screen.height;
    screen.base     = rawScreen;
  }
  
  // setup objects we need for CGImageCreate
  
  screenDataProvider = CGDataProviderCreateWithData(NULL,
                                                    screen.base,
                                                    screenBytesPerImage,
                                                    NULL);
  
  colourSpace = CGColorSpaceCreateDeviceRGB();
  
  return mmerror_OK;
  
  
failure:
  
  NSLog(@"setupMotionMaskPlot: failure: mmerr=%d", mmerr);
  
  return mmerr;
}

-(void)animate
{
  mmerror_t mmerr;
  int       i;
  
  /* clear the screen */
  
  memset32(screen.base, 0x00808080, screenBytesPerImage >> 2);
  
  /* plot the motion mask */
  
  {
    static int frame = 0;
    static int rotateSources = 0;
    
    if (rotateSources)
    {
      const bitmap_t *tmp;
      int             k;
      
      // rotate the source bitmap list such that the final image shown is used as the next start image
      
      tmp = sourceBitmapList[0];
      
      for (k = 0; k < nSourceImages - 1; k++)
        sourceBitmapList[k] = sourceBitmapList[k + 1];
      
      sourceBitmapList[k] = tmp;
      
      rotateSources = 0;
    }
    
    for (i = 0; i < 1; i++) /* loop for benchmarking */
    {
      mmerr = motionmaskplayer_plot(motionMaskPlayer,
                                    sourceBitmapList,
                                    nSourceImages,
                                    &screen,
                                    (int) mouseLocation.x,
                                    screenHeight - (int) mouseLocation.y,
                                    frame);
      if (mmerr)
        return;
    }
    
    if (++frame >= nMakerSourceImageFilenames)
    {
      frame = 0;
      rotateSources = 1;
    }
  }
  
  /* redraw just the region we've invalidated */
  
  [self setNeedsDisplayInRect:NSMakeRect(screenX,
                                         screenY,
                                         screenWidth,
                                         screenHeight)];
  // [self displayIfNeeded];
}

-(void)setTracking
{
  NSTrackingArea *trackingArea;
  
  // bounds will become stale if the window size changes (and it does)
  // i've just pumped them up to 10000,10000
  trackingArea = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0, 0, 10000, 10000)
                                              options:(NSTrackingMouseEnteredAndExited |
                                                       NSTrackingMouseMoved |
                                                       NSTrackingActiveInKeyWindow)
                                                owner:self
                                             userInfo:nil];
  [self addTrackingArea:trackingArea];
  
  [trackingArea release];
}

-(void)setTimer
{
  [NSTimer scheduledTimerWithTimeInterval:(1.0 / 30 /* fps */) target:self selector:@selector(onTick:) userInfo:nil repeats:YES];
}

-(void)onTick:(NSTimer *)timer
{
  (void) timer;
  
  [self animate];
}

// -----------------------------------------------------------------------------

// previously was implementing initWithFrame: but this is not called for interface builder objects
-(void)awakeFromNib
{
  mmerror_t mmerr;
  
  /*char buf[1000];
   getcwd(buf, 1000);
   NSLog(@"CWD is %s", buf);*/
  
  remove(motionMaskFilename);
  
  [self setTracking];
  [self setTimer];
  mmerr = [self testMotionMaskCreate:motionMaskFilename];
  if (mmerr)
    goto failure;
  
  mmerr = [self setupMotionMaskPlot:motionMaskFilename];
  if (mmerr)
    goto failure;
  
  return;
  
failure:
  
  NSLog(@"mmerr=%d in awakeFromNib", mmerr);
  
  [NSApp terminate: nil];
}

-(void)mouseEntered:(NSEvent *)theEvent
{
  (void) theEvent;
  
  [[self window] setAcceptsMouseMovedEvents:YES];
  // [self displayIfNeeded];
}

-(void)mouseExited:(NSEvent *)theEvent
{
  (void) theEvent;
  
  [[self window] setAcceptsMouseMovedEvents:NO];
  // [self displayIfNeeded];
}

-(void)mouseMoved:(NSEvent *)theEvent
{
  /* get mouse location, rounding off subpixel coordinates */
  
  mouseLocation = [self convertPoint:[theEvent locationInWindow]
                            fromView:nil];
  x = (int) floor(mouseLocation.x) - screenX;
  y = (int) floor(mouseLocation.y) - screenY;
  
  //[self animate];
}

-(void)drawRect:(NSRect)dirtyRect
{
  CGRect r;
  
  (void) dirtyRect;
  
  r = CGRectMake(screenX, screenY, screenWidth, screenHeight);
  
  // this looks like more work than is necessary just to plot a bitmap
  // can't i just kick the CGImage to update its bitmap?
  
  screenImage = CGImageCreate(screen.width, screen.height,
                              screenBPC, screenBPP,
                              screen.rowbytes,
                              colourSpace,
                              PixelfmtTobitmapInfo(screen.format),
                              screenDataProvider,
                              NULL, // decode array
                              NO, // should interpolate
                              kCGRenderingIntentDefault);
  
  CGContextDrawImage([[NSGraphicsContext currentContext] graphicsPort],
                     r,
                     screenImage);
  
  CGImageRelease(screenImage);
  screenImage = NULL;
}

-(void)dealloc
{
  int i;
  
  for (i = 0; i < nSourceImages; i++)
  {
    CGImageRelease(sourceImages[i]);
    sourceImages[i] = NULL;
  }
  
  CGDataProviderRelease(screenDataProvider);
  
  CGColorSpaceRelease(colourSpace);
  
  [super dealloc];
}

@end
