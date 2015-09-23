//
//  PlotView.m
//  MotionMasks
//
//  Created by David Thomas on 24/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#import "framebuf/screen.h"

#import "BitmapUtils.h"

#import "MotionMaskRunner.h"

#import "PlotView.h"

// -----------------------------------------------------------------------------

@interface PlotView()
{
  const screen_t   *myscreen;
  CGDataProviderRef screenDataProvider;
  CGColorSpaceRef   colourSpace;
}

@end

// -----------------------------------------------------------------------------

@implementation PlotView

// previously was implementing initWithFrame: but this is not called for interface builder objects
- (void)awakeFromNib
{
  MotionMaskRunner *runner = [MotionMaskRunner sharedInstance];

  [runner addClient:self];

  [runner configureImageSet:@"Wipe" width:640 height:480];

  [runner run];
  
  myscreen = [runner screent];

  // setup objects we need for CGImageCreate

  screenDataProvider = CGDataProviderCreateWithData(NULL,
                                                    myscreen->base,
                                                    myscreen->rowbytes * myscreen->height,
                                                    NULL);

  colourSpace = CGColorSpaceCreateDeviceRGB();

  [self setTracking];
}

- (void)dealloc
{
  CGDataProviderRelease(screenDataProvider);
  
  CGColorSpaceRelease(colourSpace);
  
  [[MotionMaskRunner sharedInstance] removeClient:self];

  [super dealloc];
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
  NSPoint mouseLocation;

  mouseLocation = [self convertPoint:[theEvent locationInWindow]
                            fromView:nil];

  [[MotionMaskRunner sharedInstance] setPosition:mouseLocation];
}

// -----------------------------------------------------------------------------

- (void)drawRect:(NSRect)dirtyRect
{
  CGImageRef screenImage;
  
  (void) dirtyRect;
  
  if (screenDataProvider == NULL)
    return;
  
  screenImage = CGImageCreate(myscreen->width, myscreen->height,
                              8, 32, // screenBPC, screenBPP,
                              myscreen->rowbytes,
                              colourSpace,
                              PixelfmtTobitmapInfo(myscreen->format),
                              screenDataProvider,
                              NULL, // decode array
                              NO, // should interpolate
                              kCGRenderingIntentDefault);
  
  CGContextDrawImage([[NSGraphicsContext currentContext] graphicsPort],
                     CGRectMake(0, 0, myscreen->width, myscreen->height),
                     screenImage);

  CGImageRelease(screenImage);
}

- (void)animationUpdated:(NSNotification*)notification
{
  NSDictionary *userInfo       = notification.userInfo;
  NSValue      *dirtyRectValue = userInfo[@"dirtyRect"];
  NSRect        dirtyRect      = dirtyRectValue.rectValue;

  [self setNeedsDisplayInRect:dirtyRect];
}

@end
