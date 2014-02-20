//
//  ProbeController.m
//  HWMonitor
//
//  Created by Kozlek on 18/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "ProbeControllingMenuItem.h"
#import "ACPIProbeArgument.h"

@implementation ProbeControllingMenuItem

- (NSUInteger)getProfileCount:(io_connect_t)connection
{
    size_t   structureInputSize;
    size_t   structureOutputSize;

    structureInputSize = sizeof(ACPIProbeArgument);
    structureOutputSize = sizeof(ACPIProbeArgument);

    ACPIProbeArgument  inputStructure;
    ACPIProbeArgument  outputStructure;

    memset(&inputStructure, 0, sizeof(ACPIProbeArgument));
    memset(&outputStructure, 0, sizeof(ACPIProbeArgument));

 	IOConnectCallStructMethod(connection, ACPIPRB_CMD_GET_PROFILE_COUNT, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize);

    return outputStructure.data_uint32;
}

- (NSString*)getProfileAtIndex:(NSUInteger)index connection:(io_connect_t)connection
{
    size_t   structureInputSize;
    size_t   structureOutputSize;

    structureInputSize = sizeof(ACPIProbeArgument);
    structureOutputSize = sizeof(ACPIProbeArgument);

    ACPIProbeArgument  inputStructure;
    ACPIProbeArgument  outputStructure;

    memset(&inputStructure, 0, sizeof(ACPIProbeArgument));
    memset(&outputStructure, 0, sizeof(ACPIProbeArgument));

    inputStructure.data_uint32 = (unsigned int)index;

 	IOConnectCallStructMethod(connection, ACPIPRB_CMD_GET_PROFILE_AT_INDEX, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize);

    return [NSString stringWithFormat:@"%s", outputStructure.data_string];
}

- (NSString*)getActiveProfile:(io_connect_t)connection
{
    size_t   structureInputSize;
    size_t   structureOutputSize;

    structureInputSize = sizeof(ACPIProbeArgument);
    structureOutputSize = sizeof(ACPIProbeArgument);

    ACPIProbeArgument  inputStructure;
    ACPIProbeArgument  outputStructure;

    memset(&inputStructure, 0, sizeof(ACPIProbeArgument));
    memset(&outputStructure, 0, sizeof(ACPIProbeArgument));

 	IOConnectCallStructMethod(connection, ACPIPRB_CMD_GET_ACTIVE_PROFILE, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize);

    return [NSString stringWithFormat:@"%s", outputStructure.data_string];
}

- (BOOL)setActiveProfile:(NSString*)profile connection:(io_connect_t)connection;
{
    size_t   structureInputSize;
    size_t   structureOutputSize;

    structureInputSize = sizeof(ACPIProbeArgument);
    structureOutputSize = sizeof(ACPIProbeArgument);

    ACPIProbeArgument  inputStructure;
    ACPIProbeArgument  outputStructure;

    memset(&inputStructure, 0, sizeof(ACPIProbeArgument));
    memset(&outputStructure, 0, sizeof(ACPIProbeArgument));

    snprintf(inputStructure.data_string, 32, "%s", profile.UTF8String);

 	if (kIOReturnSuccess == IOConnectCallStructMethod(connection, ACPIPRB_CMD_SET_ACTIVE_PROFILE, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize)) {
        if (kIOReturnSuccess == outputStructure.result) {
            return YES;
        }
    }

    return NO;
}

-(IBAction)activateProfile:(id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *)sender;

    NSDictionary *params = [sender representedObject];

    NSString *profile = params[@"profile"];
    NSNumber *connection = params[@"connection"];

    if ([self setActiveProfile:profile connection:(io_connect_t)connection.unsignedLongLongValue])
    {
        NSArray *previousItems = [_entries filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"connection == %d", connection.unsignedLongLongValue]];

        [previousItems enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSDictionary *params = (NSDictionary*)obj;

            NSMenuItem *item = params[@"item"];

            [item setState:NSOffState];
        }];

        [menuItem setState:NSOnState];
    }
}

- (void)awakeFromNib
{
    self.submenu = [[NSMenu alloc] init];

    NSMutableArray *entries = [NSMutableArray array];

    mach_port_t   masterPort;
    io_iterator_t iterator;

    IOMasterPort(MACH_PORT_NULL, &masterPort);

    CFMutableDictionaryRef matchingDictionary = IOServiceMatching("ACPIProbe");

    if (kIOReturnSuccess == IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator))
    {
        io_object_t device;
        io_connect_t connection;

        while ((device = IOIteratorNext(iterator))) {
            if (kIOReturnSuccess == IOServiceOpen(device, mach_task_self(), 0, &connection)) {

                NSUInteger count = [self getProfileCount:connection];

                if (count) {

                    // Add separator
                    if (self.submenu.itemArray.count) {
                        [self.submenu addItem:[NSMenuItem separatorItem]];
                    }

                    NSString *active = [self getActiveProfile:connection];

                    for (int index = 0; index < count; index++) {

                        NSString *profile = [self getProfileAtIndex:index connection:connection];

                        NSMenuItem *item = [self.submenu addItemWithTitle:profile action:@selector(activateProfile:) keyEquivalent:@""];

                        [entries addObject:@{@"connection": [NSNumber numberWithUnsignedLongLong:connection],
                                             @"item": item}];

                        if ([active isEqualToString:profile]) {
                            [item setState:NSOnState];
                        }
                        
                        [item setTarget:self];
                        [item setRepresentedObject:@{@"profile": profile,
                                                     @"connection": [NSNumber numberWithUnsignedLongLong:connection]}];
                    }
                }
            }
            
            IOObjectRelease(device);
        }
        
        IOObjectRelease((io_object_t)iterator);
    }

    _entries = [entries copy];



    if (self.submenu.itemArray.count) {
        [self.menu insertItem:[NSMenuItem separatorItem] atIndex:[self.menu.itemArray indexOfObject:self]];
        [self setHidden:NO];
    }
    else {
        [self setHidden:YES];
    }
}

@end
