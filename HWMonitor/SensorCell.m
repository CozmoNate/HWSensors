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
    [super setColorTheme:colorTheme];
    
    [[self textField] setTextColor:self.colorTheme.itemTitleColor];
    [[self subtitleField] setTextColor:self.colorTheme.itemSubTitleColor];
    [[self valueField] setTextColor:self.colorTheme.itemValueTitleColor];
}

@end
