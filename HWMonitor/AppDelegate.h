//
//  AppDelegate.h
//  HWMonitor
//
//  Created by kozlek on 20.10.11.
//  Copyright (c) 2011 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "HWMonitor.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    NSStatusItem *_statusItem;
    HWMonitor* _monitor;
}

@end
