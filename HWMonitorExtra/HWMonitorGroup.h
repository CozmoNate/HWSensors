//
//  HWMonitorGroup.h
//  HWSensors
//
//  Created by kozlek on 28.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorItem.h"

@interface HWMonitorGroup : NSObject
{
    HWMonitorEngine* _engine;
}

@property (readwrite, assign) enum HWSensorGroup sensorGroup;

@property (readwrite, retain) NSFont* menuFont;

@property (readwrite, retain) NSMenuItem* separatorItem;
@property (readwrite, retain) NSMenuItem* titleMenuItem;

@property (readwrite, retain) NSMutableArray* items;

@property (readwrite, assign, setter = setFirst:) BOOL isFirst;

+ (HWMonitorGroup*)groupWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu font:(NSFont*)font title:(NSString*)title image:(NSImage*)image;

- (HWMonitorGroup*)initWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu font:(NSFont*)font title:(NSString*)title image:(NSImage*)image;

- (BOOL)checkVisibility;

@end