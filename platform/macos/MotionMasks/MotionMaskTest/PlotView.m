// sloppy slapdash slipshot test app for momasks

#import <Foundation/NSTimer.h>

#import "PlotView.h"

#import "BitmapDrawing.h"
#import "BitmapTransform.h"
#import "BitmapUtils.h"
#import "ImageLoaders.h"
#import "ImageGenerators.h"
#import "Utils.h"

#import "MMCommon.h"
#import "MMMaker.h"
#import "MMPlayer.h"

#import "framebuf/screen.h"

#import "player/play.h"
#import "maker/make.h"

// -----------------------------------------------------------------------------

static const int plotOffsetX = 64;
static const int plotOffsetY = 64;

// -----------------------------------------------------------------------------

static CGDataProviderRef screenDataProvider;

static CGColorSpaceRef   colourSpace;

// -----------------------------------------------------------------------------

@interface PlotView()
{
  MMPlayer_t     *tester;
  const screen_t *screen;
  NSPoint         mouseLocation;
}

@end

// -----------------------------------------------------------------------------

@implementation PlotView

// previously was implementing initWithFrame: but this is not called for interface builder objects
- (void)awakeFromNib
{
  static const char motionMaskFilename[] = "tmp.momask";
  result_t          err;
  
  /*char buf[1000];
   getcwd(buf, 1000);
   NSLog(@"CWD is %s", buf);*/
  
  remove(motionMaskFilename); // delete any previous one kicking around
  
  err = MMMaker_make(motionMaskFilename);
  if (err)
    goto failure;

  err = MMCommon_Player_instance(&tester); // calls create
  if (err)
    goto failure;

  err = MMPlayer_setup(tester, motionMaskFilename, 640, 480);
  if (err)
    goto failure;
  
  // setup objects we need for CGImageCreate
  
  screen = MMPlayer_getScreen(tester);
  
  mouseLocation.x = plotOffsetX;
  mouseLocation.y = plotOffsetY + 480;
  
  screenDataProvider = CGDataProviderCreateWithData(NULL,
                                                    screen->base,
                                                    screen->rowbytes * screen->height,
                                                    NULL);
  
  colourSpace = CGColorSpaceCreateDeviceRGB();
  
  [self setTracking];
  [self setTimer];
  
  return;
  
failure:
  
  NSLog(@"err=%d in PlotView:awakeFromNib", err);
  
  [NSApp terminate: nil];
}

- (void)dealloc
{
  MMCommon_Player_destroy();
  
  CGDataProviderRelease(screenDataProvider);
  
  CGColorSpaceRelease(colourSpace);
  
  [super dealloc];
}

// -----------------------------------------------------------------------------

- (void)animate
{
  int x, y;
  
  x = (int) floor(mouseLocation.x) - plotOffsetX;
  y = (int) floor(mouseLocation.y) - plotOffsetY;
  
  // NSLog(@"MMPlayer_render to %d, %d", x, y);

  MMPlayer_render(tester, x, y);
  
  /* redraw just the region we've invalidated */
  [self setNeedsDisplayInRect:NSMakeRect(plotOffsetX, plotOffsetY,
                                         screen->width, screen->height)];
  //[self displayIfNeeded];
}

// -----------------------------------------------------------------------------

- (void)setTracking
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

// -----------------------------------------------------------------------------

- (void)setTimer
{
  [NSTimer scheduledTimerWithTimeInterval:1.0 / 30 /* 30fps */
                                   target:self
                                 selector:@selector(onTick:)
                                 userInfo:nil
                                  repeats:YES];
}

- (void)onTick:(NSTimer *)timer
{
  (void) timer;
  
  [self animate];
}

// -----------------------------------------------------------------------------

- (void)mouseEntered:(NSEvent *)theEvent
{
  (void) theEvent;
  
  [[self window] setAcceptsMouseMovedEvents:YES];
}

- (void)mouseExited:(NSEvent *)theEvent
{
  (void) theEvent;
  
  [[self window] setAcceptsMouseMovedEvents:NO];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
  mouseLocation = [self convertPoint:[theEvent locationInWindow]
                            fromView:nil];
}

// -----------------------------------------------------------------------------

- (void)drawRect:(NSRect)dirtyRect
{
  CGImageRef screenImage;
  
  (void) dirtyRect;
  
  if (screenDataProvider == NULL)
    return;
  
  screenImage = CGImageCreate(screen->width, screen->height,
                              8, 32, // screenBPC, screenBPP,
                              screen->rowbytes,
                              colourSpace,
                              PixelfmtTobitmapInfo(screen->format),
                              screenDataProvider,
                              NULL, // decode array
                              NO, // should interpolate
                              kCGRenderingIntentDefault);
  
  CGContextDrawImage([[NSGraphicsContext currentContext] graphicsPort],
                     CGRectMake(plotOffsetX, plotOffsetY, screen->width, screen->height),
                     screenImage);
  
  // NSLog(@"drawing at %d, %d", plotOffsetX, plotOffsetY);
  
  CGImageRelease(screenImage);
}

@end
