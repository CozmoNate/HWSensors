//
//  HWMGraph.m
//  HWMonitor
//
//  Created by Kozlek on 24/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMGraph.h"
#import "HWMSensor.h"

const NSArray * gHWMGraphsGroupColors;

@implementation HWMGraph

@dynamic color;
@dynamic identifier;
@dynamic hidden;
@dynamic sensor;

+(const NSArray*)graphColors
{
    if (!gHWMGraphsGroupColors) {

        NSMutableArray *colors = [[NSMutableArray alloc] init];

        NSColorList *list = [NSColorList colorListNamed:@"Crayons"];

        for (NSUInteger i = [[list allKeys] count] - 1; i != 0; i--) {
            NSString *key = [[list allKeys] objectAtIndex:i];
            NSColor *color = [list colorWithKey:key];
            double intensity = (color.redComponent + color.blueComponent + color.greenComponent) / 3.0;
            double red = [color redComponent];
            double green = [color greenComponent];
            double blue = [color blueComponent];
            BOOL blackAndWhite = red == green && red == blue && green == blue;

            if (intensity >= 0.335 && intensity <=0.900 && !blackAndWhite)
                [colors addObject:color];
        }

        gHWMGraphsGroupColors = [colors copy];
    }
    
    return gHWMGraphsGroupColors;
}

-(NSArray *)history
{
    return _history;
}

- (void)captureValueToHistorySetLimit:(NSUInteger)limit
{
    if (!_history) {
        _history = [[NSMutableArray alloc] init];
    }

    [_history addObject:self.sensor.value];

    if (_history.count > limit) {
        [_history removeObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _history.count - limit - 1)]];
    }
}

@end
