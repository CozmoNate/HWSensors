//
//  HWMonitorExtraView.h
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import "SystemUIPlugin.h"

@interface HWMonitorView : NSMenuExtraView

{
    NSMenuExtra *                   menu;
    
    NSImage *                       image;
    NSImage *                       altImage;
    NSArray *                       titles;
}

- (void)setTitles:(NSMutableArray*)newTitles;
- (void)setImage:(NSImage*)newImage;
- (void)setAlternateImage:(NSImage*)newAlternateImage;

@end
