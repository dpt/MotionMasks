//
//  MotionMaskGLWindowController.m
//  MotionMasks
//
//  Created by David Thomas on 24/08/2015.
//  Copyright (c) 2015 David Thomas. All rights reserved.
//

#import "MotionMaskGLWindowController.h"

@interface MotionMaskGLWindowController ()

@end

@implementation MotionMaskGLWindowController

- (id)initWithWindow:(NSWindow *)window
{
  self = [super initWithWindow:window];
  if (self)
  {
    // Initialization code here.
  }
  return self;
}

- (void)windowDidLoad
{
  [super windowDidLoad];

  // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
  NSLog(@"MotionMaskGLWindowController windowDidLoad");
}

@end
