//
//  MyOpenGLView.m
//  MotionMasks
//
//  Created by David Thomas on 24/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>

#import <GLUT/glut.h>

#import "framebuf/screen.h"

#import "MotionMaskRunner.h"

#import "MyOpenGLView.h"

#define WIDTH  640
#define HEIGHT 480

// -----------------------------------------------------------------------------

@interface MyOpenGLView()
{
  const screen_t *myscreen;
}

@end

// -----------------------------------------------------------------------------

@implementation MyOpenGLView

- (void)awakeFromNib
{
  MotionMaskRunner *runner = [MotionMaskRunner sharedInstance];

  [runner addClient:self];

  [runner configureImageSet:@"Stars" width:WIDTH height:HEIGHT];

  [runner run];

  myscreen = [runner screent];
}

- (void)dealloc
{
  [[MotionMaskRunner sharedInstance] removeClient:self];

  [super dealloc];
}

// -----------------------------------------------------------------------------

- (void)prepare
{
}

// -----------------------------------------------------------------------------

- (void)reshape
{
  [super reshape];
  
  // Convert up to window space, which is in pixel units.
  NSRect baseRect = [self convertRectToBacking:[self bounds]];
  
  // Now the result is glViewport()-compatible.

  glViewport(0, 0, (GLsizei) baseRect.size.width, (GLsizei) baseRect.size.height);
}

// -----------------------------------------------------------------------------

- (void)drawRect:(NSRect)dirtyRect
{
  (void) dirtyRect;

  // The GL context must be active for these functions to have an effect
  [[self openGLContext] makeCurrentContext];

  // Configure the view
  glShadeModel(GL_FLAT);
  glDisable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-WIDTH / 2.0, WIDTH / 2.0, -HEIGHT / 2.0, HEIGHT / 2.0); // has no effect?
//  glTranslatef(-WIDTH / 2.0, y, 0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.375, 0.375, 0);


  // Clear the background
  glClearColor(0.5f, 0.5f, 0.5f, 0.0f); // grey
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw the image
  // Subtracting 1.0f from Y is a bodge to get it to appear. If the Y hits the top of the frame then glDrawPixels skips drawing anything.
  glRasterPos2f(-WIDTH / 2.0f, (HEIGHT / 2.0f) - 1.0f);
  glPixelZoom(1.0f, -1.0f);
  glDrawPixels(myscreen->width, myscreen->height, GL_RGBA, GL_UNSIGNED_BYTE, myscreen->base);

  // Draw a Red 1x1 Square centered at origin
  glBegin(GL_QUADS); // Each set of 4 vertices form a quad
  glColor3f(1.0f, 0.0f, 0.0f); // Red
  glVertex2f(-0.5f, -0.5f); // x, y
  glVertex2f( 0.5f, -0.5f);
  glVertex2f( 0.5f,  0.5f);
  glVertex2f(-0.5f,  0.5f);
  glEnd();

  // Flush to screen
  glFinish();
}

- (void)animationUpdated:(NSNotification*)notification
{
  NSDictionary *userInfo       = notification.userInfo;
  NSValue      *dirtyRectValue = userInfo[@"dirtyRect"];
  NSRect        dirtyRect      = dirtyRectValue.rectValue;

  [self setNeedsDisplayInRect:dirtyRect];
}

@end
