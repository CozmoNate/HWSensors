//
//  ACPIProbeClient.m
//  HWMonitor
//
//  Created by Kozlek on 18/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "ACPIProbeClient.h"
#import "ACPIProbeArgument.h"

@implementation ACPIProbeClient

- (NSUInteger)getProfileCount
{
    size_t   structureInputSize;
    size_t   structureOutputSize;

    structureInputSize = sizeof(ACPIProbeArgument);
    structureOutputSize = sizeof(ACPIProbeArgument);

    ACPIProbeArgument  inputStructure;
    ACPIProbeArgument  outputStructure;

    memset(&inputStructure, 0, sizeof(ACPIProbeArgument));
    memset(&outputStructure, 0, sizeof(ACPIProbeArgument));

 	IOConnectCallStructMethod(_connection, ACPIPRB_CMD_GET_PROFILE_COUNT, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize);

    return outputStructure.data_uint32;
}

- (NSString*)getProfileAtIndex:(NSUInteger)index
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

 	IOConnectCallStructMethod(_connection, ACPIPRB_CMD_GET_PROFILE_AT_INDEX, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize);

    return [NSString stringWithFormat:@"%s", outputStructure.data_string];
}

- (NSString*)getActiveProfile
{
    size_t   structureInputSize;
    size_t   structureOutputSize;

    structureInputSize = sizeof(ACPIProbeArgument);
    structureOutputSize = sizeof(ACPIProbeArgument);

    ACPIProbeArgument  inputStructure;
    ACPIProbeArgument  outputStructure;

    memset(&inputStructure, 0, sizeof(ACPIProbeArgument));
    memset(&outputStructure, 0, sizeof(ACPIProbeArgument));

 	IOConnectCallStructMethod(_connection, ACPIPRB_CMD_GET_ACTIVE_PROFILE, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize);

    return [NSString stringWithFormat:@"%s", outputStructure.data_string];
}

- (void)setActiveProfile:(id)sender;
{
    size_t   structureInputSize;
    size_t   structureOutputSize;

    structureInputSize = sizeof(ACPIProbeArgument);
    structureOutputSize = sizeof(ACPIProbeArgument);

    ACPIProbeArgument  inputStructure;
    ACPIProbeArgument  outputStructure;

    memset(&inputStructure, 0, sizeof(ACPIProbeArgument));
    memset(&outputStructure, 0, sizeof(ACPIProbeArgument));

    snprintf(inputStructure.data_string, 32, "%s", [[sender representedObject] UTF8String]);

 	if (kIOReturnSuccess == IOConnectCallStructMethod(_connection, ACPIPRB_CMD_SET_ACTIVE_PROFILE, &inputStructure, structureInputSize, &outputStructure, &structureOutputSize)) {
        if (kIOReturnSuccess == outputStructure.result) {
            [_previousItem setState:NSOffState];
            _previousItem = sender;

            [sender setState:NSOnState];
        }
    }
}

- (id)initWithConnection:(io_connect_t)connection
{
    self = [super init];

    if (self) {

        _connection = connection;

        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength];

            NSImage *statusImage = [NSImage imageNamed:@"switch"];

            [_statusItem setImage:statusImage];
            [_statusItem setHighlightMode:YES];

            NSMenu *statusMenu = [[NSMenu alloc] init];

            NSUInteger count = [self getProfileCount];
            NSString *active = [self getActiveProfile];

            for (int index = 0; index < count; index++) {
                NSString *profile = [self getProfileAtIndex:index];

                NSMenuItem *item = [statusMenu addItemWithTitle:profile action:@selector(setActiveProfile:) keyEquivalent:@""];

                if ([active isEqualToString:profile]) {
                    [item setState:NSOnState];
                    _previousItem = item;
                }

                [item setTarget:self];
                [item setRepresentedObject:profile];
            }

            [_statusItem setMenu:statusMenu];
        }];
    }

    return self;
}

@end
