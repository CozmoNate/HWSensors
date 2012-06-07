//
//  HWMonitorView.h
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorEngine.h"

@interface HWMonitorView : NSView

{
    NSStatusItem *statusItem;
    NSFont *smallFont;
    NSFont *bigFont;
    NSShadow *shadow;
    
    bool isMenuExtra;
}

@property (readwrite, retain) NSImage *image;
@property (readwrite, retain) NSImage *alternateImage;

@property (readwrite, assign) BOOL isMenuDown;

@property (readwrite, retain) HWMonitorEngine *monitor;
@property (readwrite, retain) NSArray *favorites;

@property (readwrite, assign) BOOL drawValuesInRow;
@property (readwrite, assign) BOOL useShadowEffect;

- initWithFrame:(NSRect)rect statusItem:(NSStatusItem*)item;

@end
