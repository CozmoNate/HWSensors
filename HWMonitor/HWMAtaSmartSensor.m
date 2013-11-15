//
//  HWMAtaSmartSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMAtaSmartSensor.h"

#include <IOKit/storage/ata/ATASMARTLib.h>


@implementation HWMAtaSmartSensor

@dynamic selector;
@dynamic service;

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
