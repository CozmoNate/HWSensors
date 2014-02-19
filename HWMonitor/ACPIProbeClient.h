//
//  ACPIProbeClient.h
//  HWMonitor
//
//  Created by Kozlek on 18/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ACPIProbeClient : NSObject
{
    NSStatusItem *_statusItem;
    io_connect_t _connection;
    id _previousItem;
}

- (id)initWithConnection:(io_connect_t)connection;

@end
