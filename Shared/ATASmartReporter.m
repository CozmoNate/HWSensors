//
//  NSSmartReporter.m
//  HWSensors
//
//  Based on code by OldNavi
//
//  Created by kozlek on 19/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import "ATASmartReporter.h"

#include <IOKit/storage/ata/ATASMARTLib.h>

// NSATAGenericDisk

@implementation ATAGenericDisk

@synthesize productName;
@synthesize serialNumber;
@synthesize isRotational;
@synthesize isExceeded;

+(ATAGenericDisk*)genericDiskWithService:(io_service_t)ioservice productName:(NSString*)name serialNumber:(NSString*)serial isRotational:(BOOL)rotational
{
    if (MACH_PORT_NULL != ioservice) {
        ATAGenericDisk* me = [[ATAGenericDisk alloc] init];
        
        me->service = ioservice;

        me->productName = name;
        me->serialNumber = serial;
        me->isRotational = rotational;
        
        return me;
    }
    
    return nil;
}

-(BOOL)readSMARTData
{
    IOCFPlugInInterface ** pluginInterface = NULL;
    IOATASMARTInterface ** smartInterface = NULL;
    SInt32 score = 0;
    BOOL result = false;
    
    if (kIOReturnSuccess == IOCreatePlugInInterfaceForService(service, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score)) {
        if (S_OK == (*pluginInterface)->QueryInterface(pluginInterface, CFUUIDGetUUIDBytes( kIOATASMARTInterfaceID), (LPVOID)&smartInterface)) {
            ATASMARTData smartData;
            
            bzero(&smartData, sizeof(smartData));
            
            if(kIOReturnSuccess == (*smartInterface)->SMARTEnableDisableOperations(smartInterface, true))
                if (kIOReturnSuccess == (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, true)) {
                    
                    Boolean conditionExceeded = false;
                    
                    if (kIOReturnSuccess == (*smartInterface)->SMARTReturnStatus(smartInterface, &conditionExceeded))
                        isExceeded = conditionExceeded;
                    
                    if (kIOReturnSuccess == (*smartInterface)->SMARTReadData(smartInterface, &smartData)) 
                        if (kIOReturnSuccess == (*smartInterface)->SMARTValidateReadData(smartInterface, &smartData)) {
                            bcopy(&smartData.vendorSpecific1, &data, sizeof(data));
                            result = true;
                        }
                }
            
            (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, false);
            (*smartInterface)->SMARTEnableDisableOperations(smartInterface, false);
        }   
        
        (*smartInterface )->Release(smartInterface);
        IODestroyPlugInInterface(pluginInterface);
    }
    
    return result;
}

-(ATASMARTAttribute*)getAttributeByIdentifier:(UInt8) identifier
{
    for (int index = 0; index < kATASMARTVendorSpecificAttributesCount; index++) 
        if (data.vendorAttributes[index].attributeId == identifier)
            return &data.vendorAttributes[index];
            
    return nil;
}

-(NSData*)getTemperature
{
    if ([self readSMARTData]) {    
        ATASMARTAttribute * temperature = nil;
        
        if ((temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature]) || 
            (temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature2]))
            return [NSData dataWithBytes:&temperature->rawvalue[0] length:1];
    }
    
    return nil;
}

-(NSData*)getRemainingLife
{
    if ([self readSMARTData]) {
        ATASMARTAttribute * life = nil;
        
        if ((life = [self getAttributeByIdentifier:0xB4]) ||
            (life = [self getAttributeByIdentifier:0xD1]) ||
            (life = [self getAttributeByIdentifier:0xE8]) ||
            (life = [self getAttributeByIdentifier:0xE7]))
            return [NSData dataWithBytes:life->rawvalue length:6];
    }
    
    return nil;
}

@end

// NSATASmartReporter

@implementation NSATASmartReporter

@synthesize drives;

+(NSATASmartReporter*)smartReporterByDiscoveringDrives
{
    NSATASmartReporter* me = [[NSATASmartReporter alloc] init];
        
    if (me)
        [me diskoverDrives];
    
    return me;
}

- (void)diskoverDrives
{
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
                            
                            if (name && serial && medium) {
                                if ([medium isEqualToString:@"Rotational"]) {
                                    [list addObject:[ATAGenericDisk genericDiskWithService:service productName:name serialNumber:serial isRotational:TRUE]];
                                }
                                else if ([medium isEqualToString:@"Solid State"])
                                {
                                    [list addObject:[ATAGenericDisk genericDiskWithService:service productName:name serialNumber:serial isRotational:FALSE]];
                                }
                            }
                        }
                    }
                    
                    CFRelease(capable);
                }
            }
            
            IOObjectRelease(service);
        }
        
        IOObjectRelease(iterator);
    }
    
    drives = [NSArray arrayWithArray:list];
}

@end
