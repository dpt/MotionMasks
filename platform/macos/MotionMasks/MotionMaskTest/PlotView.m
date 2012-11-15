#import "PlotView.h"

#include "player/play.h"
#include "maker/make.h"

// TODO
// slow update speed - maybe using OpenGL would be a better idea than Cocoa (more noticable when window is large)
// free objects (leaking stuff presently)

// DONE
// images getting drawn on non-whole pixel boundarys [fixed - round mouse coords down to integers]
// mystery blue left hand column [it's gone]
// image not updating - caching first run and never refreshing [fixed - recreate objects on each draw op :-( ]

// -----------------------------------------------------------------------------

static const int screenX = 80, screenY = 80; // where to plot
static const int screenWidth = 320 * 2, screenHeight = 480;
static const int screenBPC = 8;
static const int screenBPP = 4 * screenBPC; // bits per pixel
static const int screenRowBytes = ((screenWidth * screenBPP + 127) & ~127) >> 3; // aligning to a 16-byte boundary (can be less)
static const size_t screenBufferBytes = screenRowBytes * screenHeight;

static const char motionMaskFilename[] = "tmp.momask"; // "/Users/dave/Dropbox/Projects/MoMask2/TestMask,ffd";

static const char *sourceImageFilenames[] =
{
  "/Users/dave/Google Drive/a.jpg",
  "/Users/dave/Google Drive/b.jpg",
  "/Users/dave/Google Drive/c.jpg"
};
static const int nSourceImages = 3; // NELEMS(sourceImageFilenames);

// -----------------------------------------------------------------------------

static const char *makerSourceImageFilenames[] =
{
  "/Users/dave/Google Drive/m.png"
};
static const int nMakerSourceImageFilenames = 1;

// -----------------------------------------------------------------------------

static motionmaskplayer_t  *motionMaskPlayer;
static bitmap_t             sourceBitmaps[nSourceImages];
static const bitmap_t      *sourceBitmapList[nSourceImages];
static screen_t             screen;

static CGImageRef           sourceImageRefs[nSourceImages];
static CFDataRef            sourceDataRef[nSourceImages];
static CGImageRef           screenImageRef;

static CGDataProviderRef    screenDataProviderRef;
static CGColorSpaceRef      colorSpaceRef;

static NSImage             *drawImage;

// -----------------------------------------------------------------------------

@implementation PlotView

// -----------------------------------------------------------------------------

-(CGImageRef)newJPEGFromFile:(const char *)filename
{
  CGDataProviderRef filenameDataProvider;
  CGImageRef        imageRef;
  
  filenameDataProvider = CGDataProviderCreateWithFilename(filename);
  
  imageRef = CGImageCreateWithJPEGDataProvider(filenameDataProvider,
                                               NULL,
                                               false,
                                               kCGRenderingIntentDefault);
  
  CGDataProviderRelease(filenameDataProvider);
  
  return imageRef;
}

-(CGImageRef)newPNGFromFile:(const char *)filename
{
  CGDataProviderRef filenameDataProvider;
  CGImageRef        imageRef;
  
  filenameDataProvider = CGDataProviderCreateWithFilename(filename);
  
  imageRef = CGImageCreateWithPNGDataProvider(filenameDataProvider,
                                              NULL,
                                              false,
                                              kCGRenderingIntentDefault);
  
  CGDataProviderRelease(filenameDataProvider);
  
  return imageRef;
}

// -----------------------------------------------------------------------------

static CFDataRef copyImagePixels(CGImageRef imageRef)
{
  return CGDataProviderCopyData(CGImageGetDataProvider(imageRef));
}

//static void fillWithCheckerboard(uint8_t *base, uint32_t pixel)
//{
//  unsigned int *bufi;
//  int           x,y;
//  
//  // draw a checkerboard
//  bufi = (unsigned int *) base;
//  for (y = 0; y < screenHeight; y++)
//    for (x = 0; x < screenWidth; x++)
//      bufi[(y * screenRowBytes) / 4 + x] = (x & 1) == (y & 1) ? 0x00000000 : pixel;
//}

// -----------------------------------------------------------------------------

static pixelfmt_t bitmapInfoToPixelfmt(CGBitmapInfo bitmapInfo)
{
  switch (bitmapInfo)
  {
    case kCGImageAlphaNone:
      return pixelfmt_y8;
      
    case kCGImageAlphaNoneSkipLast: // RGBX
      return pixelfmt_rgbx8888;
      
    case kCGImageAlphaNoneSkipFirst: // XRGB
      return pixelfmt_xrgb8888;
      
    default:
      NSLog(@"Unexpected CGBitmapInfo format.");
      return pixelfmt_unknown;
  }
}

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
      NSLog(@"Unexpected pixelfmt_t format.");
      return 0;
  }
}

// -----------------------------------------------------------------------------

- (CGImageRef) newGreyscaleCopy:(CGImageRef) image
{
  size_t          width, height;
  size_t          bytesPerRow;
  void           *pixels;
  CGColorSpaceRef colorSpaceRef;
  CGContextRef    contextRef;
  CGImageRef      imageRef;

  width  = CGImageGetWidth(image);
  height = CGImageGetHeight(image);
  
  bytesPerRow = ((width * 8 + 127) & ~127u) >> 3;
  
  pixels = malloc(bytesPerRow * height);
  if (pixels == NULL)
    return NULL;

  colorSpaceRef = CGColorSpaceCreateDeviceGray();
  
  contextRef = CGBitmapContextCreate(pixels,
                                     width,
                                     height,
                                     8,
                                     bytesPerRow,
                                     colorSpaceRef,
                                     kCGBitmapByteOrderDefault | kCGImageAlphaNone);
  
  CGContextDrawImage(contextRef, CGRectMake(0, 0, width, height), image);

  imageRef = CGBitmapContextCreateImage(contextRef);
  
  CGContextRelease(contextRef);
  
  CGColorSpaceRelease(colorSpaceRef);
  
  free(pixels);

  return imageRef;
}

-(void)awakeFromNib
{
  NSTrackingArea *trackingArea;
  mmerror_t       mmerr;
  int             i;
  
  // bounds will become stale if the window size changes (and it does)
  // i've just pumped them up to 10000,10000
  trackingArea = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0, 0, 10000, 10000)
                                              options:(NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow)
                                                owner:self
                                             userInfo:nil];
  [self addTrackingArea:trackingArea];
  
  [trackingArea release];
  
  
  /*char buf[1000];
  getcwd(buf, 1000);
  NSLog(@"CWD is %s", buf);*/
  
  
  
  CGBitmapInfo bitmapInfo;
  pixelfmt_t   pixelfmt;
  CFDataRef    pixels;
  


  
  //
  {
    motionmaskmaker_t *maker;
    CGImageRef         makerSource[1];
    void              *makerBitmapBases[1];
    bitmap_set_t       makerBitmaps;
    
    mmerr = motionmaskmaker_create(&maker);
    if (mmerr)
      goto failure;
    
    makerSource[0] = [self newPNGFromFile:makerSourceImageFilenames[0]];
    
    bitmapInfo = CGImageGetBitmapInfo(makerSource[0]);
    pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
    if (pixelfmt == pixelfmt_unknown)
      return;
    
    if (pixelfmt != pixelfmt_y8)
    {
      CGImageRef greycopy;
      
      greycopy = [self newGreyscaleCopy:makerSource[0]];
      
      CGImageRelease(makerSource[0]);
      
      makerSource[0] = greycopy;
      
      bitmapInfo = CGImageGetBitmapInfo(makerSource[0]);
      pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
      if (pixelfmt == pixelfmt_unknown)
        return;
    }

    makerBitmaps.width    = (int) CGImageGetWidth(makerSource[0]);
    makerBitmaps.height   = (int) CGImageGetHeight(makerSource[0]);
    makerBitmaps.format   = pixelfmt;
    makerBitmaps.rowbytes = (int) CGImageGetBytesPerRow(makerSource[0]);
    makerBitmaps.nbases   = 1;
    makerBitmaps.bases    = makerBitmapBases;
    
    pixels = copyImagePixels(makerSource[0]); // this creates a copy which will need freeing later

    makerBitmapBases[0] = (void *) CFDataGetBytePtr(pixels);
    
    mmerr = motionmaskmaker_pack(maker, &makerBitmaps);
    if (mmerr)
      goto failure;
    
    mmerr = motionmaskmaker_save(maker, "tmp.momask");
    if (mmerr)
      goto failure;

    motionmaskmaker_destroy(maker);
    maker = NULL;
  }
  
  
  
  // load mm
  
  mmerr = motionmaskplayer_create(&motionMaskPlayer);
  if (mmerr)
    goto failure;
  
  mmerr = motionmaskplayer_load(motionMaskPlayer,
                                motionMaskFilename);
  if (mmerr)
    goto failure;
  
  
  // set up sources
  // we could check that they're the same depth and dimensions but we can ignore that and use this as a test for motionmask interface itself (it ought to check)
  
  for (i = 0; i < nSourceImages; i++)
  {
    sourceImageRefs[i] = [self newJPEGFromFile:sourceImageFilenames[i]];
    
    bitmapInfo = CGImageGetBitmapInfo(sourceImageRefs[i]);
    pixelfmt = bitmapInfoToPixelfmt(bitmapInfo);
    if (pixelfmt == pixelfmt_unknown)
      return;
    
    pixels = copyImagePixels(sourceImageRefs[i]); // this creates a copy which will need freeing later
    
    sourceDataRef[i] = pixels;
    
    sourceBitmaps[i].width    = (int) CGImageGetWidth(sourceImageRefs[i]);
    sourceBitmaps[i].height   = (int) CGImageGetHeight(sourceImageRefs[i]);
    sourceBitmaps[i].format   = pixelfmt;
    sourceBitmaps[i].rowbytes = (int) CGImageGetBytesPerRow(sourceImageRefs[i]);
    sourceBitmaps[i].base     = (void *) CFDataGetBytePtr(pixels);
    
    sourceBitmapList[i] = &sourceBitmaps[i];
  }
  
  // set up screen
  
  {
    uint8_t *rawScreen;
    
    rawScreen = malloc(screenBufferBytes);
    if (rawScreen == NULL)
      goto failure;
    
    screen.width    = screenWidth;
    screen.height   = screenHeight;
    screen.format   = pixelfmt;
    screen.rowbytes = screenRowBytes;
    screen.clip.x0  = 0;
    screen.clip.y0  = 0;
    screen.clip.x1  = screen.width;
    screen.clip.y1  = screen.height;
    screen.base     = rawScreen;
  }
  
  // setup objects we need for plotting
  
  screenDataProviderRef = CGDataProviderCreateWithData(NULL,
                                                       screen.base,
                                                       screenRowBytes,
                                                       NULL);
  
  colorSpaceRef = CGColorSpaceCreateDeviceRGB();
  
  drawImage = [NSImage alloc];

  return;
  
  
failure:
  
  NSLog(@"awakeFromNib: failure: mmerr=%d", mmerr);
  
  return;
}

// previously was implementing initWithFrame: but this is not called for interface builder objects

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
  mmerror_t mmerr;
  
  // get mouse location, avoiding subpixel coordinates
  
  mouseLocation = [self convertPoint:[theEvent locationInWindow]
                            fromView:nil];
  mouseLocation.x = floor(mouseLocation.x) - screenX;
  mouseLocation.y = floor(mouseLocation.y) - screenY;
  
  mmerr = motionmaskplayer_plot(motionMaskPlayer,
                                sourceBitmapList,
                                nSourceImages,
                                &screen,
                                (int) mouseLocation.x, screenHeight - (int) mouseLocation.y,
                                0 /* frame */);
  if (mmerr)
    return;
  
  // redraw just the region we've invalidated
  [self setNeedsDisplayInRect:NSMakeRect(screenX,
                                         screenY,
                                         screenX + screenWidth,
                                         screenY + screenHeight)];
  // [self displayIfNeeded];
}

-(void)drawRect:(NSRect)dirtyRect
{
  (void) dirtyRect;
  
  // this looks like a lot of work just to plot a bitmap
  
  screenImageRef = CGImageCreate(screen.width, screen.height,
                                 screenBPC, screenBPP,
                                 screen.rowbytes,
                                 colorSpaceRef,
                                 PixelfmtTobitmapInfo(screen.format),
                                 screenDataProviderRef,
                                 NULL, // decode array
                                 NO, // should interpolate
                                 kCGRenderingIntentDefault);

  [drawImage initWithCGImage:screenImageRef size:NSZeroSize]; // NSZeroSize -> use dims of cgimage
  
  [drawImage drawAtPoint:NSMakePoint(screenX, screenY)
                fromRect:NSMakeRect(0, 0, screenWidth, screenHeight)
               operation:NSCompositeSourceOver
                fraction:1];
  
  // [drawImage recache];
}

-(void)dealloc
{
  [drawImage release];
  
  CGDataProviderRelease(screenDataProviderRef);
  
  CGColorSpaceRelease(colorSpaceRef);

  [super dealloc];
}

@end
