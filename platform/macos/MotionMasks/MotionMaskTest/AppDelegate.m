//
//  AppDelegate.m
//  MotionMaskTest
//
//  Created by David Thomas on 14/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#import "AppDelegate.h"
#import "PlotView.h"
#import "GLView.h"

/* Use class extension to hide API that doesn't yet need to be exposed.
 */
// "@interface PlotAppDelegate()" seems to stop duplicate interface defn being whined about
// this is "class extension"
@interface AppDelegate()
{
  NSWindow     *plotWindow;
  NSWindow     *glWindow;
  PlotView     *plotView;
  GLView       *glView;
  NSOpenGLView *openglView;
}

@property (assign) IBOutlet NSWindow     *plotWindow;
@property (assign) IBOutlet NSWindow     *glWindow;
@property (assign) IBOutlet PlotView     *plotView;
@property (assign) IBOutlet GLView       *glView;
@property (assign) IBOutlet NSOpenGLView *openglView;

@end

@implementation AppDelegate

@synthesize plotWindow, glWindow, plotView, glView, openglView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  (void) aNotification;
  
  [plotWindow setFrame:NSMakeRect(0, 0, 800, 600) display:YES];
  [glWindow setFrame:NSMakeRect(0, 0, 800, 600) display:YES];
  
  //[[plotWindow standardWindowButton:NSWindowCloseButton] setEnabled:NO];

  //[plotView setBounds:NSMakeRect(64, 64, 800, 600)];
}

- (void)dealloc
{
  [super dealloc];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
  (void) theApplication;
  
  return YES;
}

@end
