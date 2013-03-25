//
//  GraphsSensorCell.m
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "GraphsSensorCell.h"

@implementation GraphsSensorCell

-(void)resetCursorRects
{
    [self discardCursorRects];
    
    if (_checkBox) {
        [self addCursorRect:_checkBox.frame cursor:[NSCursor pointingHandCursor]];
    }
}

@end
