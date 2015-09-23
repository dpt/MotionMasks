//
//  MotionMaskRunner.h
//  MotionMasks
//
//  Created by David Thomas on 25/08/2015.
//  Copyright © 2015 David Thomas. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "framebuf/screen.h"

@interface MotionMaskRunner : NSObject

+ (id)sharedInstance;

- (void)addClient:(id)client;
- (void)removeClient:(id)client;

// If I call this 'screen' then stuff breaks. Unsure why.
- (const screen_t *)screent;
- (void)setPosition:(NSPoint)topLeft;
- (void)configureImageSet:(NSString *)name_in width:(int)width_in height:(int)height_in;

- (void)run;

@end
