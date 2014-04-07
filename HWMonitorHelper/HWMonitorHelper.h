//
//  HWMonitorHelper.h
//  HWMonitor
//
//  Created by Kozlek on 04.04.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

#define kHWMonitorHelperServiceName "org.hwsensors.HWMonitorHelper"

@interface HWMonitorHelper : NSObject

- (BOOL)writeNumericKey:(NSString*)key value:(NSNumber*)value;

@end
