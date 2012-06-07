//
//  HWMonitorExtra.m
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorExtra.h"

@implementation HWMonitorExtra

- (NSMenu *)menu
{
    if (_monitor)
        return [_monitor menu];
    
    return nil;
}

- (id)initWithBundle:(NSBundle *)bundle
{
    self = [super initWithBundle:bundle];
    
    if (self == nil) return nil;
    
     _monitor = [[HWMonitor alloc] initWithStatusItem:self bundle:[self bundle]];
    
    return self;
}

- (void)willUnload
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
}

@end
