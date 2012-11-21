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
}

@property (assign) IBOutlet NSWindow *plotWindow;
@property (assign) IBOutlet PlotView *plotView;

@end

@implementation AppDelegate

@synthesize plotWindow, plotView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  (void) aNotification;
  
  [plotWindow setFrame:NSMakeRect(0, 0, 800, 600) display:YES];
  //[plotView setBounds:NSMakeRect(64, 64, 800, 600)];
  [[plotWindow standardWindowButton:NSWindowCloseButton] setEnabled:NO];
}

- (void)dealloc
{
  [super dealloc];
}

@end
