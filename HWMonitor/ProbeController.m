//
//  ProbeController.m
//  HWMonitor
//
//  Created by Kozlek on 18/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "ProbeController.h"
#import "ACPIProbeClient.h"

@implementation ProbeController

- (void)awakeFromNib
{
    mach_port_t   masterPort;
    io_iterator_t iterator;

    IOMasterPort(MACH_PORT_NULL, &masterPort);

    CFMutableDictionaryRef matchingDictionary = IOServiceMatching("ACPIProbe");

    if (kIOReturnSuccess == IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator))
    {
        io_object_t device;
        io_connect_t connection;

        _clients = [[NSMutableArray alloc] init];

        while ((device = IOIteratorNext(iterator))) {

            if (kIOReturnSuccess == IOServiceOpen(device, mach_task_self(), 0, &connection)) {
                ACPIProbeClient *client = [[ACPIProbeClient alloc] initWithConnection:connection];
                [_clients addObject:client];
            }

            IOObjectRelease(device);
        }

        IOObjectRelease((io_object_t)iterator);
    }
}

@end
