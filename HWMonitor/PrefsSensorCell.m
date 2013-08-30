//
//  ItemCell.m
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PrefsSensorCell.h"

@implementation PrefsSensorCell

-(void)resetCursorRects
{
    [self discardCursorRects];
    
    [self addCursorRect:self.frame cursor:[NSCursor openHandCursor]];
    
    if (_checkBox) {
        [self addCursorRect:_checkBox.frame cursor:[NSCursor pointingHandCursor]];
    }
}

@end
