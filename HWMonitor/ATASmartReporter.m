//
//  ATASmartReporter.m
//  HWSensors
//
//  Based on ISPSmartController from iStatPro by Buffy (c) 2007
//  ISPSmartController licensed under GPL
//
//  Created by kozlek on 19/02/12.
//


#import "ATASmartReporter.h"

#include <IOKit/storage/ata/ATASMARTLib.h>

// NSATAGenericDisk

@implementation ATAGenericDisk

@synthesize productName;
@synthesize serialNumber;
@synthesize bsdName;
@synthesize volumesNames;
@synthesize isRotational;
@synthesize isExceeded;

+(ATAGenericDisk*)genericDiskWithService:(io_service_t)ioservice productName:(NSString*)name bsdName:(NSString*)bsd volumesNames:(NSString*)volumes serialNumber:(NSString*)serial isRotational:(BOOL)rotational
{
    if (MACH_PORT_NULL != ioservice) {
        ATAGenericDisk* me = [[ATAGenericDisk alloc] init];
        
        me->service = ioservice;
        
        me->productName = name;
        me->bsdName = bsd;
        me->volumesNames = volumes;
        me->serialNumber = serial;
        me->isRotational = rotational;
        
        me->lastUpdate = [NSDate dateWithTimeIntervalSinceNow:-60.0];
        
        return me;
    }
    
    return nil;
}

-(BOOL)readSMARTData
{
    if ([lastUpdate timeIntervalSinceNow] > -60.0) 
        return true;
    
    IOCFPlugInInterface ** pluginInterface = NULL;
    IOATASMARTInterface ** smartInterface = NULL;
    SInt32 score = 0;
    BOOL result = false;
    
    if (kIOReturnSuccess == IOCreatePlugInInterfaceForService(service, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score)) {
        if (S_OK == (*pluginInterface)->QueryInterface(pluginInterface, CFUUIDGetUUIDBytes(kIOATASMARTInterfaceID), (LPVOID)&smartInterface)) {
            ATASMARTData smartData;
            
            bzero(&smartData, sizeof(smartData));
            
            if(kIOReturnSuccess == (*smartInterface)->SMARTEnableDisableOperations(smartInterface, true)) {
                if (kIOReturnSuccess == (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, true)) {
                    
                    Boolean conditionExceeded = false;
                    
                    if (kIOReturnSuccess == (*smartInterface)->SMARTReturnStatus(smartInterface, &conditionExceeded))
                        isExceeded = conditionExceeded;
                    
                    if (kIOReturnSuccess == (*smartInterface)->SMARTReadData(smartInterface, &smartData)) {
                        if (kIOReturnSuccess == (*smartInterface)->SMARTValidateReadData(smartInterface, &smartData)) {
                            bcopy(&smartData.vendorSpecific1, &data, sizeof(data));
                            result = true;
                            lastUpdate = [NSDate date];
                        }
                    }
                    
                    (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, false);
                }
                
                (*smartInterface)->SMARTEnableDisableOperations(smartInterface, false);
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
        if (data.vendorAttributes[index].attributeId == identifier)
            return &data.vendorAttributes[index];
    
    return nil;
}

-(NSData*)getTemperature
{
    if ([self readSMARTData]) {    
        ATASMARTAttribute * temperature = nil;
        
        if ((temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature]) || 
            (temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature2]) ||
            (temperature = [self getAttributeByIdentifier:kATASMARTAttributeTemperature3]))
            return [NSData dataWithBytes:&temperature->rawvalue[0] length:1];
    }
    
    return nil;
}

-(NSData*)getRemainingLife
{
    if ([self readSMARTData]) {
        ATASMARTAttribute * life = nil;
        
        if ((life = [self getAttributeByIdentifier:kATASMARTAttributeEndurance]) ||
            (life = [self getAttributeByIdentifier:kATASMARTAttributeEndurance2]))
            return [NSData dataWithBytes:life->rawvalue length:6];
    }
    
    return nil;
}

-(NSData*)getRemainingBlocks
{
    if ([self readSMARTData]) {
        ATASMARTAttribute * life = nil;
        
        if ((life = [self getAttributeByIdentifier:kATASMARTAttributeUnusedReservedBloks]))
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

#include <sys/mount.h>

- (void)diskoverDrives
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
                            
                            if (name && serial) {
                                id disk = nil;
                                
                                NSString *volumes;
                                NSString *bsdName;
                                
                                CFStringRef bsdNameRef = IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);
                                
                                if (MACH_PORT_NULL != bsdNameRef) {
                                    volumes = [[partitions objectForKey:(__bridge id)(bsdNameRef)] componentsJoinedByString:@", "];
                                    bsdName = [(__bridge NSString*)bsdNameRef copy];
                                    CFRelease(bsdNameRef);
                                }
                                
                                if (medium && [medium isEqualToString:@"Solid State"])
                                {
                                    disk = [ATAGenericDisk genericDiskWithService:service productName:name bsdName:bsdName volumesNames:(volumes ? volumes : bsdName) serialNumber:serial isRotational:FALSE];
                                }
                                else /*if (medium && [medium isEqualToString:@"Rotational"]) */ {
                                    disk = [ATAGenericDisk genericDiskWithService:service productName:name bsdName:bsdName volumesNames:(volumes ? volumes : bsdName) serialNumber:serial isRotational:TRUE];
                                    ;
                                }
                                                                
                                if (disk) {
                                    [list addObject:disk];
                                }
                                else {
                                    IOObjectRelease(service);
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
        NSString *name1 = [obj1 bsdName];
        NSString *name2 = [obj2 bsdName];
        
        return [name1 compare:name2];
    }];
    
    drives = [NSArray arrayWithArray:list];
}

@end
