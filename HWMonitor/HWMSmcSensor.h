//
//  HWMSmcSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMSensor.h"


@interface HWMSmcSensor : HWMSensor

@property (nonatomic, retain) NSNumber * connection;

+ (BOOL)isValidIntegerSmcType:(NSString *)type;
+ (BOOL)isValidFloatingSmcType:(NSString *)type;
+ (float)decodeNumericValueFromBuffer:(void*)data length:(NSUInteger)length ofType:(NSString*)type;

- (void)doUpdateValue;

@end
