//
//  SmcHelper+HWMonitorHelper.h
//  HWMonitor
//
//  Created by Kozlek on 04.04.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "SmcHelper.h"

#import "HWMonitorHelper.h"

@interface SmcHelper (HWMonitorHelper)

+ (BOOL)privilegedWriteNumericKey:(NSString*)key value:(NSNumber*)value;

@end
