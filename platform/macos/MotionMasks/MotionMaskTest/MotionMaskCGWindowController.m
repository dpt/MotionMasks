//
//  MotionMaskCGWindowController.m
//  MotionMasks
//
//  Created by David Thomas on 24/08/2015.
//  Copyright (c) 2015 David Thomas. All rights reserved.
//

#import "MotionMaskCGWindowController.h"

@interface MotionMaskCGWindowController ()

@end

@implementation MotionMaskCGWindowController

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
  NSLog(@"MotionMaskCGWindowController windowDidLoad");
}

@end
