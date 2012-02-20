//
//  AppDelegate.m
//  HWMonitor
//
//  Created by mozo on 20.10.11.
//  Copyright (c) 2011 mozo. All rights reserved.
//

#import "AppDelegate.h"
#import "NSString+TruncateToWidth.h"

#include <IOKit/storage/ata/ATASMARTLib.h>
#include "FakeSMCDefinitions.h"
#include "SMART.h"

@implementation AppDelegate

- (HWMonitorSensor *)addSensorWithKey:(NSString *)key andCaption:(NSString *)caption intoGroup:(SensorGroup)group
{
    if (group == SMARTTemperatureSensorGroup || [HWMonitorSensor populateValueForKey:key]) {   
        
        caption = [caption stringByTruncatingToWidth:145.0f withFont:statusItemFont]; 
        
        HWMonitorSensor * sensor = [[HWMonitorSensor alloc] initWithKey:key andGroup:group withCaption:caption];
        
        [sensor setFavorite:[[NSUserDefaults standardUserDefaults] boolForKey:key]];
        
        NSMenuItem * menuItem = [[NSMenuItem alloc] initWithTitle:caption action:nil keyEquivalent:@""];
        
        [menuItem setRepresentedObject:sensor];
        [menuItem setAction:@selector(menuItemClicked:)];
        
        if ([sensor favorite]) [menuItem setState:TRUE];
        
        [statusMenu insertItem:menuItem atIndex:menusCount++];
        
        [sensor setObject:menuItem];
        
        [sensorsList addObject:sensor];
        
        return sensor;
    }
    
    return NULL;
}

- (void)insertFooterAndTitle:(NSString *)title
{
    if (lastMenusCount < menusCount) {
        NSMenuItem * titleItem = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
        
        [titleItem setEnabled:FALSE];
        //[titleItem setIndentationLevel:1];
        
        [statusMenu insertItem:titleItem atIndex:lastMenusCount]; menusCount++;       
        [statusMenu insertItem:[NSMenuItem separatorItem] atIndex:menusCount++];
        
        lastMenusCount = menusCount;
    }
}

- (void)updateDrivesTemperatures; 
{
    CFDictionaryRef matching = IOServiceMatching("IOBlockStorageDevice");
    io_iterator_t iterator = IO_OBJECT_NULL;
    
    if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {     
        
        if (IO_OBJECT_NULL != iterator) {
        
            io_service_t service = MACH_PORT_NULL;
            
            NSMutableDictionary * result = [[NSMutableDictionary alloc] init];
            
            while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                
                CFBooleanRef capable = (CFBooleanRef)IORegistryEntryCreateCFProperty(service, CFSTR(kIOPropertySMARTCapableKey), kCFAllocatorDefault, 0);
                
                if (capable != IO_OBJECT_NULL) {
                    if (CFBooleanGetValue(capable)) {
                        
                        NSDictionary * characteristics = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(service, CFSTR("Device Characteristics"), kCFAllocatorDefault, 0);
                        
                        if (characteristics) {
                            NSString * type = [characteristics objectForKey:@"Medium Type"];
                            
                            if (type && [type isEqualToString:@"Rotational"]) {
                                
                                NSString * name = [characteristics objectForKey:@"Product Name"];
                                
                                if (name) {
                                    
                                    /*UInt16 t = 0;
                                     
                                     [result setObject:[NSData dataWithBytes:&t length:2] forKey:name];*/
                                    
                                    IOCFPlugInInterface ** pluginInterface = NULL;
                                    IOATASMARTInterface ** smartInterface = NULL;
                                    SInt32 score = 0;
                                    HRESULT res = S_OK;
                                    
                                    if (kIOReturnSuccess == IOCreatePlugInInterfaceForService(service, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score)) {
                                        
                                        res = (*pluginInterface)->QueryInterface(pluginInterface, CFUUIDGetUUIDBytes( kIOATASMARTInterfaceID), (LPVOID)&smartInterface);
                                        
                                        if (res == S_OK) {
                                            ATASMARTData data;
                                            ATASMARTVendorSpecific1Data specific1;
                                            
                                            bzero(&data, sizeof(data));
                                            bzero(&specific1, sizeof(specific1));
                                            
                                            if(kIOReturnSuccess == (*smartInterface)->SMARTEnableDisableOperations(smartInterface, true))
                                                if (kIOReturnSuccess == (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, true))
                                                    if (kIOReturnSuccess == (*smartInterface)->SMARTReadData(smartInterface, &data)) 
                                                        if (kIOReturnSuccess == (*smartInterface)->SMARTValidateReadData(smartInterface, &data)) {
                                                            
                                                            specific1 = *((ATASMARTVendorSpecific1Data*)&(data.vendorSpecific1));
                                                            
                                                            for (int index = 0; index < kATASMARTVendorSpecific1AttributesCount; index++) {
                                                                
                                                                ATASMARTAttribute attribute = specific1.vendorAttributes[index];
                                                                
                                                                switch (attribute.attributeId) {
                                                                    case kATASMARTAttributeTemperature:
                                                                    case kATASMARTAttributeTemperature2:
                                                                        [result setObject:[NSData dataWithBytes:&attribute.rawvalue[0] length:2] forKey:name];
                                                                        break;
                                                                        
                                                                    default:
                                                                        break;
                                                                }
                                                            }
                                                        }
                                            
                                            (*smartInterface)->SMARTEnableDisableAutosave(smartInterface, false);
                                            (*smartInterface)->SMARTEnableDisableOperations(smartInterface, false);
                                        }   
                                        
                                        (*smartInterface )->Release(smartInterface);
                                        IODestroyPlugInInterface(pluginInterface);
                                    }
                                }
                            }
                        }
                    }
                    
                    CFRelease(capable);
                }
                
                IOObjectRelease(service);
            }
            
            IOObjectRelease(iterator);
            
            driveTemperatures = [NSDictionary dictionaryWithDictionary:result];
        }
    }
}

- (void)updateTitles
{
    io_service_t service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (service) {
        
        NSEnumerator * enumerator = nil;
        HWMonitorSensor * sensor = nil;
        int count = 0;
    
        CFMutableArrayRef favorites = (CFMutableArrayRef)CFArrayCreateMutable(kCFAllocatorDefault, 0, nil);
        
        enumerator = [sensorsList  objectEnumerator];
        
        while (sensor = (HWMonitorSensor *)[enumerator nextObject]) {
            if (isMenuVisible || [sensor favorite]) {
                CFTypeRef name = (CFTypeRef) CFStringCreateWithCString(kCFAllocatorDefault, [[sensor key] cStringUsingEncoding:NSUTF8StringEncoding], kCFStringEncodingUTF8);
                
                CFArrayAppendValue(favorites, name);
                
                //CFRelease(name);
            }
        }
        
        NSMutableString * statusString = [[NSMutableString alloc] init];
        
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDevicePopulateList), favorites)) 
        {           
            NSMutableDictionary * values = (__bridge_transfer NSMutableDictionary*)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
            
            if (values) {
                
                [values addEntriesFromDictionary:driveTemperatures];
                
                enumerator = [sensorsList  objectEnumerator];
                
                while (sensor = (HWMonitorSensor *)[enumerator nextObject]) {
                    
                    if (isMenuVisible) {
                        NSString * value = [sensor formateValue:[values objectForKey:[sensor key]]];
                             
                        NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:[[NSString alloc] initWithFormat:@"%S\t%S",[[sensor caption] cStringUsingEncoding:NSUTF16StringEncoding],[value cStringUsingEncoding:NSUTF16StringEncoding]] attributes:statusMenuAttributes];

                        [title addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [title length])];
                        
                        // Update menu item title
                        [(NSMenuItem *)[sensor object] setAttributedTitle:title];
                    }
                    
                    if ([sensor favorite]) {
                        NSString * value =[[NSString alloc] initWithString:[sensor formateValue:[values objectForKey:[sensor key]]]];
                        
                        [statusString appendString:@" "];
                        [statusString appendString:value];
                        
                        count++;
                    }
                }
            }
        }
        
        CFArrayRemoveAllValues(favorites);
        CFRelease(favorites);
        
        IOObjectRelease(service);
        
        if (count > 0) {
            // Update status bar title
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:statusString attributes:statusItemAttributes];
            [title addAttribute:NSFontAttributeName value:statusItemFont range:NSMakeRange(0, [title length])];
            [statusItem setAttributedTitle:title];
        }
        else [statusItem setTitle:@""];
    }
}

// Events

- (void)menuWillOpen:(NSMenu *)menu {
    isMenuVisible = YES;
    
    [self updateTitles];
}

- (void)menuDidClose:(NSMenu *)menu {
    isMenuVisible = NO;
}

- (void)menuItemClicked:(id)sender {
    NSMenuItem * menuItem = (NSMenuItem *)sender;
    
    [menuItem setState:![menuItem state]];
    
    HWMonitorSensor * sensor = (HWMonitorSensor *)[menuItem representedObject];
    
    [sensor setFavorite:[menuItem state]];
    
    [self updateTitles];
    
    [[NSUserDefaults standardUserDefaults] setBool:[menuItem state] forKey:[sensor key]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Main sensors timer
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:
                                [self methodSignatureForSelector:@selector(updateTitles)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateTitles)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:2.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    [self updateTitles];
    
    // Main SMART timer
    invocation = [NSInvocation invocationWithMethodSignature:
                  [self methodSignatureForSelector:@selector(updateDrivesTemperatures)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateDrivesTemperatures)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:300.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
}

- (void)awakeFromNib
{
    menusCount = 0;
    
    statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    [statusItem setMenu:statusMenu];
    [statusItem setHighlightMode:YES];
    [statusItem setImage:[NSImage imageNamed:@"thermo"]];
    [statusItem setAlternateImage:[NSImage imageNamed:@"thermotemplate"]];
    
    statusItemFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0];
    
    NSMutableParagraphStyle * style = [[NSMutableParagraphStyle alloc] init];
    [style setLineSpacing:0];

    statusItemAttributes = [NSDictionary dictionaryWithObject:style forKey:NSParagraphStyleAttributeName];
    
    statusMenuFont = [NSFont fontWithName:@"Lucida Grande Bold" size:10];
    [statusMenu setFont:statusMenuFont];
    
    style = [[NSMutableParagraphStyle alloc] init];
    [style setTabStops:[NSArray array]];
    [style addTabStop:[[NSTextTab alloc] initWithType:NSRightTabStopType location:190.0]];
    [style setLineBreakMode:NSLineBreakByTruncatingTail];

    statusMenuAttributes = [NSDictionary dictionaryWithObject:style forKey:NSParagraphStyleAttributeName];
    
    // Init sensors
    sensorsList = [[NSMutableArray alloc] init];
    lastMenusCount = menusCount;
    
    //Temperatures
    
    for (int i=0; i<0xA; i++)
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TC%XD",i] andCaption:[[NSString alloc] initWithFormat:@"CPU %X",i] intoGroup:TemperatureSensorGroup];
    
    [self addSensorWithKey:@"Th0H" andCaption:@"CPU Heatsink" intoGroup:TemperatureSensorGroup];
    [self addSensorWithKey:@"TN0P" andCaption:@"Motherboard" intoGroup:TemperatureSensorGroup];
    [self addSensorWithKey:@"TA0P" andCaption:@"Ambient" intoGroup:TemperatureSensorGroup];
    
    for (int i=0; i<0xA; i++) {
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TG%XD",i] andCaption:[[NSString alloc] initWithFormat:@"GPU %X Core",i] intoGroup:TemperatureSensorGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TG%XH",i] andCaption:[[NSString alloc] initWithFormat:@"GPU %X Board",i] intoGroup:TemperatureSensorGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TG%XP",i] andCaption:[[NSString alloc] initWithFormat:@"GPU %X Proximity",i] intoGroup:TemperatureSensorGroup];
    }
    
    [self insertFooterAndTitle:@"TEMPERATURES"];
    
    // Hard Drive Temperatures

    [self updateDrivesTemperatures];
    
    if (driveTemperatures) {
        NSEnumerator * enumerator = [driveTemperatures keyEnumerator];
        
        NSString * key;
        
        while (key = (NSString*)[enumerator nextObject])
            [self addSensorWithKey:key andCaption:key intoGroup:SMARTTemperatureSensorGroup];
    }
    
    [self insertFooterAndTitle:@"HARD DRIVE TEMPERATURES"];
    
    //Multipliers
    
    for (int i=0; i<0xA; i++)
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"MC%XC",i] andCaption:[[NSString alloc] initWithFormat:@"CPU %X",i] intoGroup:MultiplierSensorGroup];
    
    [self addSensorWithKey:@"MPkC" andCaption:@"CPU Package" intoGroup:MultiplierSensorGroup];
    
    [self insertFooterAndTitle:@"MULTIPLIERS"];
    
    // Fans
    
    for (int i=0; i<10; i++) {
        NSString * caption = [[NSString alloc] initWithData:[HWMonitorSensor populateValueForKey:[[NSString alloc] initWithFormat:@"F%XID",i] ]encoding: NSUTF8StringEncoding];
        if ([caption length]<=0) 
            caption = [[NSString alloc] initWithFormat:@"Fan %d",i];
        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"F%XAc",i] andCaption:caption intoGroup:TachometerSensorGroup ];
    }
    
    [self insertFooterAndTitle:@"FANS"];
    
    // Voltages

    [self addSensorWithKey:@"VC0C" andCaption:@"CPU Voltage" intoGroup:VoltageSensorGroup];
    [self addSensorWithKey:@"VM0R" andCaption:@"DIMM Voltage" intoGroup:VoltageSensorGroup];
    
    [self insertFooterAndTitle:@"VOLTAGES"];
    
    if (![sensorsList count]) {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:@"No sensors found or FakeSMCDevice unavailable" action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [statusMenu insertItem:item atIndex:0];
    }
}

@end
