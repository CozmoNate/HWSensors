//
//  HWMAtaSmartSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
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

#import "HWMAtaSmartSensor.h"

#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensorsGroup.h"

#include <sys/mount.h>

static NSMutableDictionary * gIOCFPlugInInterfaces;

@implementation HWMSmartPlugInInterfaceWrapper

+(HWMSmartPlugInInterfaceWrapper*)wrapperWithService:(io_service_t)service forBsdName:(NSString*)name
{

    if (!gIOCFPlugInInterfaces) {
        gIOCFPlugInInterfaces = [[NSMutableDictionary alloc] init];
    }

    HWMSmartPlugInInterfaceWrapper *wrapper = [gIOCFPlugInInterfaces objectForKey:name];

    if (!wrapper) {
        IOCFPlugInInterface ** pluginInterface = NULL;
        SInt32 score = 0;

        wrapper = [[HWMSmartPlugInInterfaceWrapper alloc] init];

        if (S_OK == IOCreatePlugInInterfaceForService(service, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score)) {

            (*pluginInterface)->AddRef(pluginInterface);
            [wrapper setPluginInterface:pluginInterface];

            IOATASMARTInterface ** smartInterface = NULL;

            if (S_OK == (*wrapper.pluginInterface)->QueryInterface(wrapper.pluginInterface, CFUUIDGetUUIDBytes(kIOATASMARTInterfaceID), (LPVOID)&smartInterface)) {

                [wrapper setSmartInterface:smartInterface];

                [gIOCFPlugInInterfaces setObject:wrapper forKey:name];
            }
            else {
                if (smartInterface) {
                    (*pluginInterface)->Release(pluginInterface);
                    (*smartInterface)->Release(smartInterface);
                    IODestroyPlugInInterface(pluginInterface);
                    wrapper = nil;
                }
            }
        }
        else {
            wrapper = nil;
        }
    }

    return wrapper;
}

+(HWMSmartPlugInInterfaceWrapper*)getWrapperForBsdName:(NSString*)name
{
    if (name &&gIOCFPlugInInterfaces) {
        return [gIOCFPlugInInterfaces objectForKey:name];
    }

    return nil;
}

+(void)destroyAllWrappers
{
    if (gIOCFPlugInInterfaces) {
        [gIOCFPlugInInterfaces enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
            [obj destroy];
        }];

        [gIOCFPlugInInterfaces removeAllObjects];
        gIOCFPlugInInterfaces = nil;
    }
}

-(void)destroy
{
    if (self.smartInterface) {
        (*self.smartInterface)->Release(self.smartInterface);
    }

    if (self.pluginInterface) {
        (*self.pluginInterface)->Release(self.pluginInterface);

        IODestroyPlugInInterface(self.pluginInterface);
    }
}

@end

const UInt8 kATASMARTAttributeTemperature           = 0xC2;
const UInt8 kATASMARTAttributeTemperature2          = 0xE7;
const UInt8 kATASMARTAttributeTemperature3          = 0xBE;
const UInt8 kATASMARTAttributeEndurance             = 0xE8;
const UInt8 kATASMARTAttributeEndurance2            = 0xE7;
const UInt8 kATASMARTAttributeUnusedReservedBloks   = 0xB4;

@implementation HWMAtaSmartSensor

@dynamic productName;
@dynamic bsdName;
@dynamic volumeNames;
@dynamic rotational;

@synthesize exceeded = _exceeded;

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
                            //                            NSString *revision = [characteristics objectForKey:@"Product Revision Level"];

                            if (name && serial) {
                                NSString *volumes;
                                NSString *bsdName;

                                CFStringRef bsdNameRef = IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);

                                if (MACH_PORT_NULL != bsdNameRef) {
                                    volumes = [[partitions objectForKey:(__bridge id)(bsdNameRef)] componentsJoinedByString:@", "];
                                    bsdName = [(__bridge NSString*)bsdNameRef copy];
                                    CFRelease(bsdNameRef);
                                }

                                if (bsdName) {

                                    if ([HWMSmartPlugInInterfaceWrapper wrapperWithService:service forBsdName:bsdName]) {

                                        [list addObject:@{@"service" : [NSNumber numberWithUnsignedLongLong:service],
                                                          @"productName": name,
                                                          @"bsdName" :bsdName,
                                                          @"volumesNames" : (volumes ? volumes : bsdName) ,
                                                          @"serialNumber" : serial,
                                                          @"rotational" : [NSNumber numberWithBool:medium ? [medium isEqualToString:@"Solid State"] : TRUE]}
                                         ];
                                    }
                                }
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

-(void)prepareForDeletion
{
    IOObjectRelease((io_service_t)self.service.unsignedLongLongValue);
}

-(NSString*)stringFromError:(unsigned int)errorVal
{
    NSDictionary *ioReturnMap =
    @{@kIOReturnSuccess:          @"success",
       @kIOReturnError:            @"general error",
       @kIOReturnNoMemory:         @"memory allocation error",
       @kIOReturnNoResources:      @"resource shortage",
       @kIOReturnIPCError:         @"Mach IPC failure",
       @kIOReturnNoDevice:         @"no such device",
       @kIOReturnNotPrivileged:    @"privilege violation",
       @kIOReturnBadArgument:      @"invalid argument",
       @kIOReturnLockedRead:       @"device is read locked",
       @kIOReturnLockedWrite:      @"device is write locked",
       @kIOReturnExclusiveAccess:  @"device is exclusive access",
       @kIOReturnBadMessageID:     @"bad IPC message ID",
       @kIOReturnUnsupported:      @"unsupported function",
       @kIOReturnVMError:          @"virtual memory error",
       @kIOReturnInternalError:    @"internal driver error",
       @kIOReturnIOError:          @"I/O error",
       @kIOReturnCannotLock:       @"cannot acquire lock",
       @kIOReturnNotOpen:          @"device is not open",
       @kIOReturnNotReadable:      @"device is not readable",
       @kIOReturnNotWritable:      @"device is not writeable",
       @kIOReturnNotAligned:       @"alignment error",
       @kIOReturnBadMedia:         @"media error",
       @kIOReturnStillOpen:        @"device is still open",
       @kIOReturnRLDError:         @"rld failure",
       @kIOReturnDMAError:         @"DMA failure",
       @kIOReturnBusy:             @"device is busy",
       @kIOReturnTimeout:          @"I/O timeout",
       @kIOReturnOffline:          @"device is offline",
       @kIOReturnNotReady:         @"device is not ready",
       @kIOReturnNotAttached:      @"device/channel is not attached",
       @kIOReturnNoChannels:       @"no DMA channels available",
       @kIOReturnNoSpace:          @"no space for data",
       @kIOReturnPortExists:       @"device port already exists",
       @kIOReturnCannotWire:       @"cannot wire physical memory",
       @kIOReturnNoInterrupt:      @"no interrupt attached",
       @kIOReturnNoFrames:         @"no DMA frames enqueued",
       @kIOReturnMessageTooLarge:  @"message is too large",
       @kIOReturnNotPermitted:     @"operation is not permitted",
       @kIOReturnNoPower:          @"device is without power",
       @kIOReturnNoMedia:          @"media is not present",
       @kIOReturnUnformattedMedia: @"media is not formatted",
       @kIOReturnUnsupportedMode:  @"unsupported mode",
       @kIOReturnUnderrun:         @"data underrun",
       @kIOReturnOverrun:          @"data overrun",
       @kIOReturnDeviceError:      @"device error",
       @kIOReturnNoCompletion:     @"no completion routine",
       @kIOReturnAborted:          @"operation was aborted",
       @kIOReturnNoBandwidth:      @"bus bandwidth would be exceeded",
       @kIOReturnNotResponding:    @"device is not responding",
       @kIOReturnInvalid:          @"unanticipated driver error",
       @0:                         @"0"};

    return [ioReturnMap objectForKey:[NSNumber numberWithInt:errorVal]];
}

-(BOOL)readSMARTData
{
    if (updated && [updated timeIntervalSinceNow] > -60.0)
        return YES;

    IOReturn result = kIOReturnError;

    HWMSmartPlugInInterfaceWrapper *wrapper = [HWMSmartPlugInInterfaceWrapper getWrapperForBsdName:self.bsdName];

    if (wrapper) {

            ATASMARTData smartData;

            bzero(&smartData, sizeof(smartData));

            _exceeded = false;

            if (kIOReturnSuccess != (*wrapper.smartInterface)->SMARTReturnStatus(wrapper.smartInterface, &_exceeded)) {
                if (kIOReturnSuccess != (*wrapper.smartInterface)->SMARTEnableDisableOperations(wrapper.smartInterface, true)) {
                    result = (*wrapper.smartInterface)->SMARTEnableDisableAutosave(wrapper.smartInterface, true);
                }
            }

            if (kIOReturnSuccess == (result = (*wrapper.smartInterface)->SMARTReturnStatus(wrapper.smartInterface, &_exceeded))) {
                if (kIOReturnSuccess == (result = (*wrapper.smartInterface)->SMARTReadData(wrapper.smartInterface, &smartData))) {
                    if (kIOReturnSuccess == (result = (*wrapper.smartInterface)->SMARTValidateReadData(wrapper.smartInterface, &smartData))) {
                        bcopy(&smartData.vendorSpecific1, &_smartData, sizeof(_smartData));
                        updated = [NSDate date];
                    }
                }
            }
    }

    if (result != kIOReturnSuccess)
        NSLog(@"readSMARTData returned error '%@' for device %@", [self stringFromError:result], self.bsdName);

    return result == kIOReturnSuccess;
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
            UInt64 value =  (UInt64)life->rawvalue[0] << 40 |
                            (UInt64)life->rawvalue[1] << 32 |
                            (UInt64)life->rawvalue[2] << 24 |
                            (UInt64)life->rawvalue[3] << 16 |
                            (UInt64)life->rawvalue[4] << 8 |
                            (UInt64)life->rawvalue[5];
            return [NSNumber numberWithUnsignedLong:value];
        }
    }

    return nil;
}

-(NSNumber*)getRemainingBlocks
{
    if ([self readSMARTData]) {
        ATASMARTAttribute * life = nil;

        if ((life = [self getAttributeByIdentifier:kATASMARTAttributeUnusedReservedBloks])) {
            UInt64 value =  (UInt64)life->rawvalue[0] << 40 |
                            (UInt64)life->rawvalue[1] << 32 |
                            (UInt64)life->rawvalue[2] << 24 |
                            (UInt64)life->rawvalue[3] << 16 |
                            (UInt64)life->rawvalue[4] << 8 |
                            (UInt64)life->rawvalue[5];
            return [NSNumber numberWithUnsignedLong:value];
        }
    }

    return nil;
}

-(NSUInteger)internalUpdateAlarmLevel
{
    float floatValue = self.value.floatValue;

    switch (self.selector.unsignedIntegerValue) {

        case kHWMGroupSmartTemperature:
            if (self.rotational.boolValue) {
                return  floatValue >= 55 ? kHWMSensorLevelExceeded :
                        floatValue >= 50 ? kHWMSensorLevelHigh :
                        floatValue >= 40 ? kHWMSensorLevelModerate :
                        kHWMSensorLevelNormal;
            }
            else {
                return  floatValue >= 100 ? kHWMSensorLevelExceeded :
                        floatValue >= 85 ? kHWMSensorLevelHigh :
                        floatValue >= 70 ? kHWMSensorLevelModerate :
                        kHWMSensorLevelNormal;
            }

        case kHWMGroupSmartRemainingLife:
            return  floatValue >= 90 ? kHWMSensorLevelExceeded :
                    floatValue >= 80 ? kHWMSensorLevelHigh :
                    floatValue >= 70 ? kHWMSensorLevelModerate :
                    kHWMSensorLevelNormal;

        default:
            break;
    }

    return _alarmLevel;
}

-(NSNumber *)internalUpdateValue
{
    switch (self.selector.unsignedIntegerValue) {
        case kHWMGroupSmartTemperature:
            return [self getTemperature];

        case kHWMGroupSmartRemainingLife:
            return [self getRemainingLife];

        case kHWMGroupSmartRemainingBlocks:
            return [self getRemainingBlocks];
    }

    return @0;
}

@end
