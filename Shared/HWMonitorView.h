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
    NSStatusItem *_statusItem;
    
    NSFont *_smallFont;
    NSFont *_bigFont;
    NSShadow *_shadow;
    
    bool _isMenuExtra;
}

@property (readwrite, retain) NSImage *image;
@property (readwrite, retain) NSImage *alternateImage;

@property (readwrite, assign) BOOL isMenuDown;

@property (readwrite, retain) HWMonitorEngine *engine;
@property (readwrite, retain) NSArray *favorites;

@property (readwrite, assign) BOOL useBigFont;
@property (readwrite, assign) BOOL useShadowEffect;

- initWithFrame:(NSRect)rect statusItem:(NSStatusItem*)item;

@end
