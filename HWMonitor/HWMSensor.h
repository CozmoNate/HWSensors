//
//  HWMSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

#import "HWMItem.h"

@class HWMGroup;
@class HWMEngine;

@interface HWMSensor : HWMItem

@property (nonatomic, retain) NSNumber * level;
@property (nonatomic, retain) NSNumber * order;
@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) NSString * type;
@property (nonatomic, retain) NSNumber * value;

@property (nonatomic, retain) HWMGroup * group;

@property (nonatomic, strong) HWMEngine * engine;

@property (readonly) NSString * formattedValue;

@end
