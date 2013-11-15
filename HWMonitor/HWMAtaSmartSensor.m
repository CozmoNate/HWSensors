//
//  HWMAtaSmartSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMAtaSmartSensor.h"

#include <IOKit/storage/ata/ATASMARTLib.h>
#include <sys/mount.h>

@implementation HWMAtaSmartSensor

@dynamic selector;
@dynamic service;
@dynamic productName;
@dynamic bsdName;
@dynamic volumeNames;
@dynamic serialNumber;
@dynamic rotational;
@dynamic exceeded;

+(NSArray*)discoverDrives
{
    NSMutableDictionary *partitions = [[NSMutableDictionary alloc] init];

    NSString *path;
	BOOL first = YES;

    NSEnumerator *mountedPathsEnumerator = [[[NSWorkspace  sharedWorkspace] mountedLocalVolumePaths] objectEnumerator];

    while (path = [mountedPathsEnumerator nextObject] )
    {
		struct statfs buffer;

        if (statfs([path fileSystemRepresentation],&buffer) == 0)
        {
			NSRange start = [path rangeOfString:@"/Volumes/"];

			if (first == NO && start.length == 0)
            {
				continue;
			}

			if (first)
				first = NO;

			NSString *name = [[NSString stringWithFormat:@"%s",buffer.f_mntfromname] lastPathComponent];

			if ([name hasPrefix:@"disk"] && [name length] > 4)
            {
				NSString *newName = [name substringFromIndex:4];
				NSRange paritionLocation = [newName rangeOfString:@"s"];

				if(paritionLocation.length != 0)
					name = [NSString stringWithFormat:@"disk%@",[newName substringToIndex: paritionLocation.location]];
			}

			if( [partitions objectForKey:name] )
				[[partitions objectForKey:name] addObject:[[NSFileManager defaultManager] displayNameAtPath:path]];
            else
				[partitions setObject:[[NSMutableArray alloc] initWithObjects:[[NSFileManager defaultManager] displayNameAtPath:path], nil] forKey:name];
		}
	}

    NSMutableArray * list = [[NSMutableArray alloc] init];

    CFDictionaryRef matching = IOServiceMatching("IOBlockStorageDevice");
    io_iterator_t iterator = IO_OBJECT_NULL;

    if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
        if (IO_OBJECT_NULL != iterator) {

            io_service_t service = MACH_PORT_NULL;

            while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {

                CFBooleanRef capable = (CFBooleanRef)IORegistryEntryCreateCFProperty(service, CFSTR(kIOPropertySMARTCapableKey), kCFAllocatorDefault, 0);

                if (capable != IO_OBJECT_NULL) {
                    if (CFBooleanGetValue(capable)) {

                        NSDictionary * characteristics = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(service, CFSTR("Device Characteristics"), kCFAllocatorDefault, 0);

                        if (characteristics) {
                            NSString *name = [characteristics objectForKey:@"Product Name"];
                            NSString *serial = [characteristics objectForKey:@"Serial Number"];
                            NSString *medium = [characteristics objectForKey:@"Medium Type"];
                            NSString *revision = [characteristics objectForKey:@"Product Revision Level"];

                            if (name && (serial || revision)) {
                                NSString *volumes;
                                NSString *bsdName;

                                CFStringRef bsdNameRef = IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);

                                if (MACH_PORT_NULL != bsdNameRef) {
                                    volumes = [[partitions objectForKey:(__bridge id)(bsdNameRef)] componentsJoinedByString:@", "];
                                    bsdName = [(__bridge NSString*)bsdNameRef copy];
                                    CFRelease(bsdNameRef);
                                }

                                [list addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                                                 [NSNumber numberWithUnsignedLongLong:service], @"service",
                                                 name, @"productName",
                                                 bsdName, @"bsdName",
                                                 (volumes ? volumes : bsdName), @"volumesNames",
                                                 serial, @"serialNumber",
                                                 [NSNumber numberWithBool:medium ? [medium isEqualToString:@"Solid State"] : TRUE], @"rotational",
                                                 nil]];

                            }
                        }
                    }

                    CFRelease(capable);
                }
            }

            IOObjectRelease(iterator);
        }
    }

    [list sortUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        NSString *name1 = [(NSDictionary*)obj1 objectForKey:@"bsdName"];
        NSString *name2 = [(NSDictionary*)obj2 objectForKey:@"bsdName"];

        return [name1 compare:name2];
    }];

    return list;
}

-(void)dealloc
{
    IOObjectRelease((io_service_t)self.service.unsignedLongLongValue);
}

-(BOOL)readSMARTData
{
    if (updated && [updated timeIntervalSinceNow] > -60.0)
        return YES;

    IOCFPlugInInterface ** pluginInterface = NULL;
    IOATASMARTInterface ** smartInterface = NULL;
    SInt32 score = 0;

    BOOL result = NO;

    if (kIOReturnSuccess == IOCreatePlugInInterfaceForService((io_service_t)self.service.unsignedLongLongValue, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score)) {
        if (S_OK == (*pluginInterface)->QueryInterface(pluginInterface, CFUUIDGetUUIDBytes(kIOATASMARTInterfaceID), (LPVOID)&smartInterface)) {
            ATASMARTData smartData;

            bzero(&smartData, sizeof(smartData));

            Boolean conditionExceeded = false;

            if (kIOReturnSuccess != (*smartInterface)->SMARTReturnStatus(smartInterface, &conditionExceeded)) {
                if (kIOReturnSuccess != (*smartInterface)->SMARTEnableDisableOperations(smartInterface, true) ||
                    kIOReturnSuccess != (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, true)) {
                    result = NO;
                }
            }

            if (kIOReturnSuccess == (*smartInterface)->SMARTReturnStatus(smartInterface, &conditionExceeded)) {

                exceeded = conditionExceeded;

                if (kIOReturnSuccess == (*smartInterface)->SMARTReadData(smartInterface, &smartData)) {
                    if (kIOReturnSuccess == (*smartInterface)->SMARTValidateReadData(smartInterface, &smartData)) {
                        bcopy(&smartData.vendorSpecific1, &_smartData, sizeof(_smartData));
                        updated = [NSDate date];
                        result = YES;
                    }
                }
            }

            (*smartInterface)->Release(smartInterface);
        }

        IODestroyPlugInInterface(pluginInterface);
    }

    return result;
}

-(ATASMARTAttribute*)getAttributeByIdentifier:(UInt8) identifier
{
    for (int index = 0; index < kATASMARTVendorSpecificAttributesCount; index++)
        if (_smartData.vendorAttributes[index].attributeId == identifier)
            return &_smartData.vendorAttributes[index];

    return nil;
}

-(NSNumber*)getTemperature
{
    if ([self readSMARTData]) {
        ATASMARTAttribute * temperature = nil;

        if ((temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature]) ||
            (temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature2]) ||
            (temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature3]))
            return [NSNumber numberWithUnsignedChar:temperature->rawvalue[0]];
    }

    return nil;
}

-(NSNumber*)getRemainingLife
{
    if ([self readSMARTData]) {
        ATASMARTAttribute * life = nil;

        if ((life = [self getAttributeByIdentifier:kATASMARTAttributeEndurance]) ||
            (life = [self getAttributeByIdentifier:kATASMARTAttributeEndurance2])) {
            UInt64 value = *((UInt64*)life->rawvalue);
            return [NSNumber numberWithUnsignedLong:value >> 16];
        }
    }

    return nil;
}

-(NSNumber*)getRemainingBlocks
{
    if ([self readSMARTData]) {
        ATASMARTAttribute * life = nil;

        if ((life = [self getAttributeByIdentifier:kATASMARTAttributeUnusedReservedBloks])) {
            UInt64 value = *((UInt64*)life->rawvalue);
            return [NSNumber numberWithUnsignedLong:value >> 16];
        }
    }

    return nil;
}

-(void)doUpdateValue
{
    switch (self.selector.unsignedIntegerValue) {
        case kHWMAtaSmartSensorTypeTemperature:
            [self willChangeValueForKey:@"data"];
            [self setPrimitiveValue:[self getTemperature] forKey:@"data"];
            [self didChangeValueForKey:@"data"];
            break;
            
        case kHWMAtaSmartSensorTypeLife:
            [self willChangeValueForKey:@"data"];
            [self setPrimitiveValue:[self getRemainingLife] forKey:@"data"];
            [self didChangeValueForKey:@"data"];
            break;
            
        case kHWMAtaSmartSensorTypeBlocks:
            [self willChangeValueForKey:@"data"];
            [self setPrimitiveValue:[self getRemainingBlocks] forKey:@"data"];
            [self didChangeValueForKey:@"data"];
            break;
            
        default:
            break;
    }
}

@end
