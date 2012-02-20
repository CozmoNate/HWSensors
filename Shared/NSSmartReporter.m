//
//  NSSmartReporter.m
//  HWSensors
//
//  Created by Natan Zalkin on 19/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#import "NSSmartReporter.h"

#include <IOKit/storage/ata/ATASMARTLib.h>

@implementation NSGenericDisk

+(NSGenericDisk*)genericDiskWithService:(io_service_t) s
{
    if (MACH_PORT_NULL != s) {
        NSGenericDisk* me = [[NSGenericDisk alloc] init];
        
        me->service = s;
        
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
                if (kIOReturnSuccess == (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, true))
                    if (kIOReturnSuccess == (*smartInterface)->SMARTReadData(smartInterface, &smartData)) 
                        if (kIOReturnSuccess == (*smartInterface)->SMARTValidateReadData(smartInterface, &smartData)) {
                            bcopy(&smartData.vendorSpecific1, &data, sizeof(data));
                            result = true;
                        }
            
            (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, false);
            (*smartInterface)->SMARTEnableDisableOperations(smartInterface, false);
        }   
        
        (*smartInterface )->Release(smartInterface);
        IODestroyPlugInInterface(pluginInterface);
    }
    
    return result;
}

-(ATASMARTAttribute*)getSMARTAttributeByIdentifier:(UInt8) identifier
{
    for (int index = 0; index < kATASMARTVendorSpecificAttributesCount; index++) 
        if (data.vendorAttributes[index].attributeId == identifier)
            return &data.vendorAttributes[index];
            
    return nil;
}

@end

@implementation NSSmartReporter

- (NSDictionary*)diskoverDrives
{
    //NSMutableDictionary * result = [NSMutableDictionary dictionary];
    
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
                            
                            NSString * value = nil;
                            
                            value = [characteristics objectForKey:@"Medium Type"];
                            
                            if (value) {
                                
                            }
                            
                            value = [characteristics objectForKey:@"Product Name"];
                            
                            if (value) {
                                
                            }
                        }
                    }
                }
                
                CFRelease(capable);
            }
            
            IOObjectRelease(service);
        }
        
        IOObjectRelease(iterator);
    }
    
    //services
    
    return nil;
}

@end
