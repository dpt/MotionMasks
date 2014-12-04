//
//  MyOpenGLView.m
//  MotionMasks
//
//  Created by David Thomas on 24/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#import <CoreFoundation/CFDate.h>

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>

#import <GLUT/glut.h>

#import "framebuf/screen.h"

#import "MMCommon.h"
#import "MMPlayer.h"

#import "MyOpenGLView.h"

// -----------------------------------------------------------------------------

@interface MyOpenGLView()
{
  MMPlayer_t *tester;
}

@end

// -----------------------------------------------------------------------------

@implementation MyOpenGLView

- (void)awakeFromNib
{
  result_t err;

  err = MMCommon_Player_instance(&tester); // calls create
  if (err)
    goto failure;
  
  [self setTimer];
  
  return;
  
failure:
  
  NSLog(@"err=%d in MyOpenGLView:awakeFromNib", err);
  
  [NSApp terminate: nil];
}

- (void)dealloc
{
  MMCommon_Player_destroy();
  
  [super dealloc];
}

// -----------------------------------------------------------------------------

- (id)initWithCoder:(NSCoder *)c
{
  self = [super initWithCoder:c];
  if (self) {
    [self prepare];
  }
  return self;
}

// -----------------------------------------------------------------------------

- (void)prepare
{
  NSLog(@"prepare");
  
  // The GL context must be active for these functions to have an effect
  [[self openGLContext] makeCurrentContext];
  
  // Configure the view
  glShadeModel(GL_FLAT);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, 0, 1);
  glDisable(GL_DEPTH_TEST);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.375, 0.375, 0);
}

// -----------------------------------------------------------------------------

- (void)animate
{
  /*int x, y;
  
  x = (int) floor(mouseLocation.x) - plotOffsetX;
  y = (int) floor(mouseLocation.y) - plotOffsetY;
  
  MMPlayer_render(tester, x, y);*/

  /* redraw just the region we've invalidated */
  [self setNeedsDisplayInRect:NSMakeRect(0, 0, 1000, 1000)];
}

- (void)reshape
{  
  // Convert up to window space, which is in pixel units.
  NSRect baseRect = [self convertRectToBase:[self bounds]];
  
  // Now the result is glViewport()-compatible.
  glViewport(0, 0, (GLsizei) baseRect.size.width, (GLsizei) baseRect.size.height);
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

- (void)drawRect:(NSRect)dirtyRect
{
  const screen_t *screen;
  
  (void) dirtyRect;
  
  screen = MMPlayer_getScreen(tester);
  
  // Clear the background
  glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  
  // Draw the image
  glRasterPos2i(0, 0);
  glPixelZoom(1.0f, -1.0f);
  glDrawPixels(screen->width, screen->height, GL_RGBA, GL_UNSIGNED_BYTE, screen->base);
  
  // Flush to screen
  glFinish();
}

@end
