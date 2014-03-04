//
//  HWMSmcFanControlLevel.h
//  HWMonitor
//
//  Created by Kozlek on 03/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMSmcFanController;

@interface HWMSmcFanControlLevel : NSManagedObject

@property (nonatomic, retain) NSNumber * input;
@property (nonatomic, retain) NSNumber * output;
@property (nonatomic, retain) HWMSmcFanController *controller;
@property (nonatomic, retain) HWMSmcFanControlLevel *next;
@property (nonatomic, retain) HWMSmcFanControlLevel *previous;

@property (readonly) NSNumber * minInput;
@property (readonly) NSNumber * maxInput;
@property (readonly) NSNumber * minOutput;
@property (readonly) NSNumber * maxOutput;

-(void)removeThisLevel;
-(HWMSmcFanControlLevel*)insertNextLevel;

@end
