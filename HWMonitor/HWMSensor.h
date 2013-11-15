//
//  HWMSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMGroup;

@interface HWMSensor : NSManagedObject

@property (nonatomic, retain) NSNumber * data;
@property (nonatomic, retain) NSNumber * hidden;
@property (nonatomic, retain) NSString * key;
@property (nonatomic, retain) NSString * legend;
@property (nonatomic, retain) NSString * title;
@property (nonatomic, retain) NSString * type;
@property (nonatomic, retain) NSNumber * order;
@property (nonatomic, retain) NSNumber * level;
@property (nonatomic, retain) HWMGroup * group;

//- (void)doUpdateValue;

@end
