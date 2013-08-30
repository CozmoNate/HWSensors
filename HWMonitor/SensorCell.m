//
//  HWMonitorSensorCell.m
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "SensorCell.h"

@implementation SensorCell

//-(void)resetCursorRects
//{
//    [self discardCursorRects];
//    //[self addCursorRect:self.bounds cursor:[NSCursor openHandCursor]];
//
//    [self addCursorRect:self.frame cursor:[NSCursor openHandCursor]];
//}

-(void)setColorTheme:(ColorTheme *)colorTheme
{
    _colorTheme = colorTheme;
    
    [[self textField] setTextColor:_colorTheme.itemTitleColor];
    [[self subtitleField] setTextColor:_colorTheme.itemSubTitleColor];
    [[self valueField] setTextColor:_colorTheme.itemValueTitleColor];
}

@end
