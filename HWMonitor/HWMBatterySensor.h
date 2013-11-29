//
//  HWMBatterySensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMSensor.h"

@interface HWMBatterySensor : HWMSensor

@property (nonatomic, retain) NSString * productName;
@property (nonatomic, retain) NSString * serialNumber;
@property (nonatomic, retain) NSNumber * selector;

+(NSArray*)discoverDevices;

-(void)doUpdateValue;

@end
