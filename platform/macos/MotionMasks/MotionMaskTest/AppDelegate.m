//
//  AppDelegate.m
//  MotionMaskTest
//
//  Created by David Thomas on 14/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#import "AppDelegate.h"
#import "PlotView.h"

// "@interface PlotAppDelegate()" seems to stop duplicate interface defn being whined about
// this is "class extension"
@interface AppDelegate()
{
  NSWindow *plotWindow;
  PlotView *plotView;
  NSImage  *sourceImage;
}

@property (assign) IBOutlet NSWindow *plotWindow;
@property (assign) IBOutlet PlotView *plotView;
@property (retain) NSImage *sourceImage;

@end

@implementation AppDelegate

@synthesize plotWindow, plotView, sourceImage;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  (void) aNotification;
  
  [plotWindow setFrame: NSMakeRect(0, 0, 800, 600) display:YES];
  [[plotWindow standardWindowButton:NSWindowCloseButton] setEnabled:NO];
}

- (void)dealloc
{
  [super dealloc];
}

@end
