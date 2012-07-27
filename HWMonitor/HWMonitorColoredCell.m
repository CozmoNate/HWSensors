//
//  HWMonitorColoredCell.m
//  HWSensors
//
//  Created by kozlek on 09.07.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
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
