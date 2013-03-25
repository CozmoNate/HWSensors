//
//  ItemCell.m
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PrefsCell.h"

@implementation PrefsCell

-(void)resetCursorRects
{
    [self discardCursorRects];
    [self addCursorRect:self.bounds cursor:[NSCursor openHandCursor]];
    
    if (_checkBox) {
        [self addCursorRect:_checkBox.frame cursor:[NSCursor pointingHandCursor]];
    }
}

@end
