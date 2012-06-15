//
//  HWMonitorExtra.h
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "HWMonitor.h"

@interface HWMonitorExtra : NSMenuExtra
{
    NSStatusItem *_statusItem;
    HWMonitor* _monitor;
}

@end
