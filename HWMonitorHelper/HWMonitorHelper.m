//
//  HWMonitorHelper.m
//  HWMonitor
//
//  Created by Kozlek on 04.04.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "HWMonitorHelper.h"
#import "SmcHelper.h"
#import <syslog.h>

static io_connect_t gSmcConnection = 0;

@implementation HWMonitorHelper

+ (io_connect_t)globalConnection
{
    if (!gSmcConnection) {
        SMCOpen("AppleSMC", &gSmcConnection);
    }

    return gSmcConnection;
}

- (BOOL)writeNumericKey:(NSString*)key value:(NSNumber*)value
{
    syslog(LOG_NOTICE, "writing key %s value %f", key.UTF8String, value.floatValue);
    
    return [SmcHelper writeNumericKey:key value:value connection:[HWMonitorHelper globalConnection]];
}

@end
