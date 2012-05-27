//
//  HWMonitorExtraView.h
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import "HWMonitorEngine.h"

@interface HWMonitorCustomView : NSView

{
    NSStatusItem *statusItem;
    NSFont *smallFont;
    NSFont *bigFont;
    NSShadow *shadow;
}

@property (readwrite, retain) NSImage *image;
@property (readwrite, retain) NSImage *alternateImage;

@property (readwrite, assign) BOOL isMenuDown;

@property (readwrite, retain) HWMonitorEngine *monitor;
@property (readwrite, retain) NSArray *favorites;

@property (readwrite, assign) BOOL drawValuesInRow;

- initWithFrame:(NSRect)rect statusItem:(NSStatusItem*)item;
- (void)setIsMenuDown:(BOOL)down;
- (BOOL)isMenuDown;

@end
