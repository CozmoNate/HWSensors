//
//  GraphsSensorCell.m
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import "GraphsSensorCell.h"

#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMGraph.h"
#import "HWMSensor.h"

#import <ReactiveCocoa/ReactiveCocoa.h>

@implementation GraphsSensorCell

-(id)init
{
    self = [super init];

    if (self) {
        [self initialize];
    }

    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self) {
        [self initialize];
    }

    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

    if (self) {
        [self initialize];
    }

    return self;
}

-(void)initialize
{
    [[RACObserve(self, objectValue)
      filter:^BOOL(id value) {
        return value != nil;
      }]
      subscribeNext:^(HWMGraph *graph) {
          [RACObserve(graph.sensor, alarmLevel)
           subscribeNext:^(NSNumber *level) {
               switch (level.integerValue) {
                    case kHWMSensorLevelNormal:
                       [self.valueField setTextColor:[NSColor whiteColor]];
                       break;

                    case kHWMSensorLevelModerate:
                       [self.valueField setTextColor:[NSColor colorWithCalibratedRed:0.7f green:0.3f blue:0.03f alpha:1.0f]];
                       break;

                    case kHWMSensorLevelHigh:
                       [self.valueField setTextColor:[NSColor redColor]];
                       break;

                    case kHWMSensorLevelExceeded:
                       [self.textField setTextColor:[NSColor redColor]];
                       [self.valueField setTextColor:[NSColor redColor]];
                       break;
               }
           }];
      }];
}

-(void)resetCursorRects
{
    [self discardCursorRects];

    //[self addCursorRect:self.frame cursor:[NSCursor openHandCursor]];

    if (_checkBox) {
        [self addCursorRect:_checkBox.frame cursor:[NSCursor pointingHandCursor]];
    }
}

@end
