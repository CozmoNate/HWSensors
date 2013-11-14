//
//  HWMSensor.h
//  HWMonitor
//
//  Created by Kozlek on 14.11.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMGroup;

@interface HWMSensor : NSManagedObject

@property (nonatomic, retain) NSString * key;
@property (nonatomic, retain) NSString * type;
@property (nonatomic) BOOL hidden;
@property (nonatomic, retain) NSString * legend;
@property (nonatomic) float value;
@property (nonatomic, retain) NSString * title;
@property (nonatomic, retain) HWMGroup *group;

@end
