//
//  MotionMaskRunner.m
//  MotionMasks
//
//  Created by David Thomas on 25/08/2015.
//  Copyright © 2015 David Thomas. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "framebuf/screen.h"

#import "player/play.h"
#import "maker/make.h"

#import "MMSinglePlayer.h"
#import "MMMaker.h"
#import "MMPlayer.h"

#import "MotionMaskRunner.h"

// -----------------------------------------------------------------------------

#define VERBOSE 0

// -----------------------------------------------------------------------------

@interface MotionMaskRunner()
{
  MMPlayer_t *player;
  NSPoint     plotCoord;
  NSString   *animationName; /* name of the animation to load */
  int         playerWidth, playerHeight;
  bool        running;
  box_t       previousDirtyBox;
}

@end

// -----------------------------------------------------------------------------

@implementation MotionMaskRunner

#pragma mark Singleton Methods

+ (id)sharedInstance
{
  static MotionMaskRunner *sharedInstance = nil;
  static dispatch_once_t   onceToken = 0;

  // This technique is faster than @synchronized.
  dispatch_once(&onceToken, ^{
    sharedInstance = [[self alloc] init];
  });

  return sharedInstance;
}

- (id)init
{
  if (self = [super init])
  {
    player = NULL;
    plotCoord = NSMakePoint(0.0, 0.0);
    animationName = @"Stars";
    playerWidth = 320;
    playerHeight = 240;
    running = false;
    box_reset(&previousDirtyBox);
  }

  return self;
}

- (void)dealloc
{
  [self cleanupMotionMasks];

  // [NSNotificationCenter dealloc]; Is this correct?

  [super dealloc];
}

#pragma mark Observers

- (void)addClient:(id)client
{
  // Register this client for dirty rectangle updates.
  [[NSNotificationCenter defaultCenter] addObserver:client
                                           selector:@selector(animationUpdated:)
                                               name:nil
                                             object:self];
}

- (void)removeClient:(id)client
{
  // Deregister this client.
  [[NSNotificationCenter defaultCenter] removeObserver:client];
}

- (void)informClients:(NSRect)dirtyRect
{
  NSDictionary *userInfo;

  userInfo = @{ @"dirtyRect": [NSValue valueWithRect:dirtyRect] }; // can't seem to @(box) NSRects

  // Notify every client about the dirty rectangle.
  [[NSNotificationCenter defaultCenter] postNotificationName:@"not-used"
                                                      object:self
                                                    userInfo:userInfo];
}

#pragma mark Accessors

- (const screen_t *)screent
{
  return MMPlayer_getScreen(player);
}

- (void)setPosition:(NSPoint)topLeft
{
  // Do y-inversion here. Since this is a .m file it can be part of the
  // Objective-C coordinate world.
  plotCoord.x = topLeft.x;
  plotCoord.y = playerHeight - topLeft.y;
}

#pragma mark Configure

- (void)configureImageSet:(NSString *)name_in width:(int)width_in height:(int)height_in
{
  animationName = name_in;
  playerWidth   = width_in;
  playerHeight  = height_in;
}

#pragma mark Loader

static const char temporaryMotionMaskFilename[] = "tmp.momask";

- (void)run
{
  result_t    err;
  const char *sourceFramesDirs[1];
  const char *sourceImagesDirs[1];

  // Should I be using a semaphore here?
  if (running)
    return;

  running = true;

  // Test code to print the CWD.
  if (VERBOSE)
  {
    char buf[1000];
    getcwd(buf, 1000);
    NSLog(@"CWD is %s", buf);
  }

  [self cleanupMotionMasks];

  // Find resources in bundle.
  //

  NSString *sourceFramesDir = [[NSBundle mainBundle] pathForResource:animationName
                                                              ofType:nil];
  if (sourceFramesDir == nil)
  {
    err = result_NOT_FOUND;
    goto failure;
  }

  NSString *sourceImagesDir = [[NSBundle mainBundle] pathForResource:@"images"
                                                              ofType:nil];
  if (sourceImagesDir == nil)
  {
    err = result_NOT_FOUND;
    goto failure;
  }

  sourceFramesDirs[0] = [sourceFramesDir UTF8String];
  sourceImagesDirs[0] = [sourceImagesDir UTF8String];

  // Build the motion mask.
  err = MMMaker_make(&sourceFramesDirs[0], 1, temporaryMotionMaskFilename);
  if (err)
    goto failure;

  // Obtain and setup a player.
  //

  err = MMSinglePlayer_instance(&player);
  if (err)
    goto failure;

  err = MMPlayer_setup(player,
                       temporaryMotionMaskFilename,
                       playerWidth,
                       playerHeight,
                       sourceImagesDirs,
                       1);
  if (err)
    goto failure;

  // Set the animation going.
  [self setTimer];

  return;


failure:

  NSLog(@"err=%d in PlotView:awakeFromNib", err);

  [NSApp terminate: nil];
}

- (void)cleanupMotionMasks
{
  MMSinglePlayer_destroy();

  remove(temporaryMotionMaskFilename);
}

#pragma mark Animation driver

- (void)setTimer
{
  const double fps = 1.0 / 30.0; /* 30fps */
  NSTimer     *timer;

  timer = [NSTimer scheduledTimerWithTimeInterval:fps
                                           target:self
                                         selector:@selector(onTick:)
                                         userInfo:nil
                                          repeats:YES];

  [timer setTolerance:fps * 2.0];
}

- (void)onTick:(NSTimer *)timer
{
  box_t  dirtyBox;
  box_t  dirtyUnion;
  NSRect flippedRect;

  (void) timer;

  if (VERBOSE)
    NSLog(@"MMPlayer_render to %f, %f", plotCoord.x, plotCoord.y);

  MMPlayer_render(player, (int) plotCoord.x, (int) plotCoord.y, &dirtyBox);

  // The first returned rectangle should be the full frame. Subsequent ones
  // should be the union of the previous and current dirty rects so that
  // movement does not leave trails.
  //

  if (box_is_empty(&previousDirtyBox))
  {
    // previousDirtyBox is in invalid initial state. Return the full frame.
    previousDirtyBox.x0 = 0;
    previousDirtyBox.y0 = 0;
    previousDirtyBox.x1 = playerWidth;
    previousDirtyBox.y1 = playerHeight;
  }

  box_union(&previousDirtyBox, &dirtyBox, &dirtyUnion);

  // Flip the dirty rect here for return.
  flippedRect.origin.x    = dirtyUnion.x0;
  flippedRect.origin.y    = playerHeight - dirtyUnion.y1;
  flippedRect.size.width  = dirtyUnion.x1 - dirtyUnion.x0;
  flippedRect.size.height = dirtyUnion.y1 - dirtyUnion.y0;

  if (VERBOSE)
    NSLog(@"rect: (%.f,%.f) %.f x %.f",
          flippedRect.origin.x, flippedRect.origin.y,
          flippedRect.size.width, flippedRect.size.height);

  // Inform all clients of the dirtied rectangle.
  [self informClients:flippedRect];

  // Save dirtyBox for next time.
  previousDirtyBox = dirtyBox;
}

@end
