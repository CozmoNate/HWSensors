//
//  HWMonitorItem.h
//  HWSensors
//
//  Created by kozlek on 28.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorSensor.h"

@class HWMonitorGroup;

@interface HWMonitorItem : NSObject
{
    HWMonitorGroup* _group;
}

@property (readwrite, retain) NSString* title;

@property (readwrite, retain) HWMonitorSensor* sensor;
@property (readwrite, retain) NSMenuItem* menuItem;

@property (readwrite, assign, setter = setVisible:) BOOL isVisible;
@property (readwrite, assign, setter = setFavorite:) BOOL isFavorite;

+ (HWMonitorItem*)itemWithGroup:(HWMonitorGroup*)group sensor:(HWMonitorSensor*)sensor menuItem:(NSMenuItem*)menuItem;

- (HWMonitorItem*)initWithGroup:(HWMonitorGroup*)group sensor:(HWMonitorSensor*)sensor menuItem:(NSMenuItem*)menuItem;

@end