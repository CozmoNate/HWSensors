//
//  SmcHelper+HWMonitorHelper.h
//  HWMonitor
//
//  Created by Kozlek on 22/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "SmcHelper.h"

@interface SmcHelper (HWMonitorHelper)

+ (void)privilegedWriteNumericKey:(NSString*)key value:(NSNumber*)value;

@end
