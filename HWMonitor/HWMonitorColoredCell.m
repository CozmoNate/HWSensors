//
//  HWMonitorColoredCell.m
//  HWSensors
//
//  Created by Kozlek on 09.07.12.
//
//

#import "HWMonitorColoredCell.h"

@implementation HWMonitorColoredCell

-(BOOL)drawsBackground
{
    return YES;
}

-(void)setTextColor:(NSColor *)color
{
    [super setTextColor:color];
    [super setBackgroundColor:color];
}

@end
