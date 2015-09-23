//
//  AppDelegate.m
//  MotionMaskTest
//
//  Created by David Thomas on 14/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#import "AppDelegate.h"

#import "MotionMaskCGWindowController.h"
#import "MotionMaskGLWindowController.h"
#import "PlotView.h"

/* Use class extension to hide API that doesn't yet need to be exposed.
 */
// "@interface PlotAppDelegate()" seems to stop duplicate interface defn being whined about
// this is "class extension"
@interface AppDelegate()
{
  PlotView     *plotView;
  NSOpenGLView *openglView;
}

@property (assign) IBOutlet MotionMaskCGWindowController *motionMaskCGWindowController;
@property (assign) IBOutlet MotionMaskGLWindowController *motionMaskGLWindowController;

@property (assign) IBOutlet PlotView     *plotView;
@property (assign) IBOutlet NSOpenGLView *openglView;

@end

@implementation AppDelegate

@synthesize motionMaskCGWindowController, motionMaskGLWindowController, plotView, openglView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  (void) aNotification;
  
  //[plotWindow setFrame:NSMakeRect(0, 0, 800, 600) display:YES];
  //[glWindow setFrame:NSMakeRect(0, 0, 800, 600) display:YES];

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
  
  return NO;
}

- (IBAction)motionMaskCGClicked:(id)sender
{
  (void) sender;

  if (!motionMaskCGWindowController)
    motionMaskCGWindowController = [[MotionMaskCGWindowController alloc] initWithWindowNibName:@"MotionMaskCGWindowController"];
  [motionMaskCGWindowController showWindow:self];
}

- (IBAction)motionMaskGLClicked:(id)sender
{
  (void) sender;

  if (!motionMaskGLWindowController)
    motionMaskGLWindowController = [[MotionMaskGLWindowController alloc] initWithWindowNibName:@"MotionMaskGLWindowController"];
  [motionMaskGLWindowController showWindow:self];
}

@end
