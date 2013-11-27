//
//  HWMGraph.h
//  HWMonitor
//
//  Created by Kozlek on 24/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMSensor;

@interface HWMGraph : NSManagedObject
{
    @private
    NSMutableArray *_history;
}

@property (nonatomic, retain) NSColor *color;
@property (nonatomic, retain) NSString * identifier;
@property (nonatomic, retain) NSNumber * hidden;
@property (nonatomic, retain) HWMSensor *sensor;

@property (readonly) NSArray *history;

+ (const NSArray*)graphColors;
- (void)captureValueToHistorySetLimit:(NSUInteger)limit;

@end
