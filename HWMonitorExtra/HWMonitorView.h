//
//  HWMonitorExtraView.h
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import "SystemUIPlugin.h"
#import "HWMonitorEngine.h"

@interface HWMonitorView : NSMenuExtraView

{
    NSMenuExtra *menu;
    NSFont *smallFont;
    NSFont *bigFont;
    NSShadow *shadow;
}

@property (readwrite, retain) HWMonitorEngine *monitor;
@property (readwrite, retain) NSArray *favorites;

@property (readwrite, assign) BOOL drawValuesInRow;

@end
