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
    NSStatusItem *menu;
    NSFont *font;
    NSShadow *shadow;
}

@property (readwrite, retain) NSImage *image;
@property (readwrite, retain) NSImage *alternateImage;

@property (readwrite, assign) BOOL isMenuDown;
@property (readwrite, retain) HWMonitorEngine *monitor;
@property (readwrite, retain) NSArray *favorites;

- initWithFrame:(NSRect)rect statusItem:(NSStatusItem*)item;

@end
