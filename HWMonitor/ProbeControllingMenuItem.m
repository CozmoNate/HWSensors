//
//  ProbeController.m
//  HWMonitor
//
//  Created by Kozlek on 18/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

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

    NSDictionary *parameters = [sender representedObject];

    NSString *profile = parameters[@"profile"];
    NSNumber *connection = parameters[@"connection"];

    if ([self setActiveProfile:profile connection:(io_connect_t)connection.unsignedLongLongValue])
    {
        NSArray *previousItems = [_entries filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"connection == %d", connection.unsignedLongLongValue]];

        [previousItems enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSDictionary *itemParams = (NSDictionary*)obj;
            NSMenuItem *item = itemParams[@"item"];
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
        //[self.menu insertItem:[NSMenuItem separatorItem] atIndex:[self.menu.itemArray indexOfObject:self]];
        [self setHidden:NO];
    }
    else {
        [self setHidden:YES];
    }
}

@end
