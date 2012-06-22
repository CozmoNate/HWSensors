//
//  HWMonitorView.h
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "SystemUIPlugin.h"
#import "HWMonitorEngine.h"

@interface HWMonitorView : NSMenuExtraView
{
    NSFont *_smallFont;
    NSFont *_bigFont;
    NSShadow *_shadow;
}

@property (readwrite, retain) HWMonitorEngine *engine;
@property (readwrite, retain) NSArray *favorites;

@property (readwrite, assign) BOOL useBigFont;
@property (readwrite, assign) BOOL useShadowEffect;

@end
