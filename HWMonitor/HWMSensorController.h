//
//  HWMSensorController.h
//  HWMonitor
//
//  Created by Kozlek on 01/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMSensor;

@interface HWMSensorController : NSManagedObject

@property (nonatomic, retain) NSNumber * enabled;
@property (nonatomic, retain) HWMSensor *output;
@property (nonatomic, retain) HWMSensor *input;

@end
