//
//  HWMonitorExtra.m
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import "HWMonitorExtra.h"
#import "HWMonitorView.h"
#import "NSString+TruncateToWidth.h"

#include <IOKit/storage/ata/ATASMARTLib.h>
#include "FakeSMCDefinitions.h"

@implementation HWMonitorExtra

- (HWMonitorSensor *)addSensorWithKey:(NSString *)key andCaption:(NSString *)caption intoGroup:(SensorGroup)group
{
    if (group == SMARTTemperatureSensorGroup || group == SMARTRemainingLifeSensorGroup || [HWMonitorSensor populateValueForKey:key]) {
        
        caption = [caption stringByTruncatingToWidth:130.0f withFont:statusBarFont]; 
        
        HWMonitorSensor * sensor = [[HWMonitorSensor alloc] initWithKey:key andGroup:group withCaption:caption];
        
        [sensor setFavorite:[[NSUserDefaults standardUserDefaults] boolForKey:key]];
        
        NSMenuItem * menuItem = [[NSMenuItem alloc] initWithTitle:caption action:@selector(menuItemClicked:) keyEquivalent:@""];
        
        [menuItem setTarget:self];
        [menuItem setRepresentedObject:sensor];
        
        if ([sensor favorite]) [menuItem setState:TRUE];
        
        [menu insertItem:menuItem atIndex:menusCount++];
        
        [sensor setObject:menuItem];
        
        [sensorsList addObject:sensor];
        
        return sensor;
    }
    
    return NULL;
}

- (void)insertFooterAndTitle:(NSString *)title
{
    if (lastMenusCount < menusCount) {
        NSMutableAttributedString * atributedTitle = [[NSMutableAttributedString alloc] initWithString:title attributes:statusMenuAttributes];
        
        [atributedTitle addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [title length])];
        
        NSMenuItem * titleItem = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
        
        [titleItem setEnabled:FALSE];
        [titleItem setAttributedTitle:atributedTitle];
        
        [menu insertItem:titleItem atIndex:lastMenusCount]; 
        menusCount++;
        
        if (lastMenusCount > 0) {
            [menu insertItem:[NSMenuItem separatorItem] atIndex:lastMenusCount];
            menusCount++;
        }
        
        lastMenusCount = menusCount;
    }
}

- (void)updateSMARTData; 
{
    if ([smartReporter drives]) {
        NSMutableDictionary * temperatures = [[NSMutableDictionary alloc] init];
        NSMutableDictionary * lifes = [[NSMutableDictionary alloc] init];
        
        NSEnumerator *enumerator = [[smartReporter drives] keyEnumerator];
        
        NSString *key;
        
        while (key = (NSString*)[enumerator nextObject]) {
            
            NSATAGenericDisk *disk = [[smartReporter drives] objectForKey:key];
            
            if (disk) {
                if ([disk isRotational]) {
                    ATASMARTAttribute * temperature = nil;
                    
                    [disk readSMARTData];
                    
                    if ((temperature = [disk getSMARTAttributeByIdentifier:kATASMARTAttributeTemperature]) || 
                        (temperature = [disk getSMARTAttributeByIdentifier:kATASMARTAttributeTemperature2]))
                        [temperatures setObject:[NSData dataWithBytes:&temperature->rawvalue[0] length:2] forKey:key];
                }
                else {
                    ATASMARTAttribute * life = nil;
                    
                    [disk readSMARTData];
                    
                    if ((life = [disk getSMARTAttributeByIdentifier:0xB4]) ||
                        (life = [disk getSMARTAttributeByIdentifier:0xD1]) ||
                        (life = [disk getSMARTAttributeByIdentifier:0xE8]) ||
                        (life = [disk getSMARTAttributeByIdentifier:0xE7]))
                        [lifes setObject:[NSData dataWithBytes:&life->rawvalue[0] length:2] forKey:key];
                }
            }
        }
        
        driveTemperatures = [NSDictionary dictionaryWithDictionary:temperatures];
        driveRemainingLifes = [NSDictionary dictionaryWithDictionary:lifes];
    }
}

- (void)updateTitles:(BOOL)force
{
    io_service_t service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (service) {
        CFMutableArrayRef list = (CFMutableArrayRef)CFArrayCreateMutable(kCFAllocatorDefault, 0, nil);
        
        NSEnumerator * enumerator = [sensorsList  objectEnumerator];
        
        HWMonitorSensor * sensor = nil;
        
        while (sensor = (HWMonitorSensor *)[enumerator nextObject]) {
            if (force || [self isMenuDown] || [sensor favorite]) {
                CFTypeRef name = (CFTypeRef) CFStringCreateWithCString(kCFAllocatorDefault, [[sensor key] cStringUsingEncoding:NSUTF8StringEncoding], kCFStringEncodingUTF8);
                
                CFArrayAppendValue(list, name);
                
                //CFRelease(name);
            }
        }
        
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDevicePopulateList), list)) 
        {           
            NSMutableDictionary * values = (__bridge_transfer NSMutableDictionary *)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
            
            if (!values) 
                values = [[NSMutableDictionary alloc] init];
            
            if (values) {
                
                [values addEntriesFromDictionary:driveTemperatures];
                [values addEntriesFromDictionary:driveRemainingLifes];
                
                NSMutableArray * favorites = [[NSMutableArray alloc] init];
                
                enumerator = [sensorsList  objectEnumerator];
                
                while (sensor = (HWMonitorSensor *)[enumerator nextObject]) {
                    if (force || [self isMenuDown] || [sensor favorite]) {
                        
                        NSString * value = [sensor formateValue:[values objectForKey:[sensor key]]];
                        
                        if (force || [self isMenuDown]) {
                            NSString * value = [sensor formateValue:[values objectForKey:[sensor key]]];

                            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:[[NSString alloc] initWithFormat:@"%S\t%S",[[sensor caption] cStringUsingEncoding:NSUTF16StringEncoding],[value cStringUsingEncoding:NSUTF16StringEncoding]] attributes:statusMenuAttributes];
                            
                            [title addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [title length])];
                            
                            // Update menu item title
                            [(NSMenuItem *)[sensor object] setAttributedTitle:title];
                        }
                        
                        if ([sensor favorite])
                            [favorites addObject:[[NSString alloc] initWithFormat:@"%S", [value cStringUsingEncoding:NSUTF16StringEncoding]]];
                    }
                }
        
                if ([favorites count] > 0)
                    [view setTitles:favorites];
                else 
                    [view setTitles:nil];
            }
        }
        
        CFArrayRemoveAllValues(list);
        CFRelease(list);
        
        IOObjectRelease(service);
    }
}

- (void)updateTitlesForced
{
    [self updateTitles:YES];
}

- (void)updateTitlesDefault
{
    [self updateTitles:NO];
}

- (void)menuItemClicked:(id)sender 
{
    NSMenuItem * menuItem = (NSMenuItem *)sender;
    
    [menuItem setState:![menuItem state]];
    
    HWMonitorSensor * sensor = (HWMonitorSensor *)[menuItem representedObject];
    
    [sensor setFavorite:[menuItem state]];
    
    [self updateTitlesDefault];
    
    [[NSUserDefaults standardUserDefaults] setBool:[menuItem state] forKey:[sensor key]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (id)initWithBundle:(NSBundle *)bundle
{
    self = [super initWithBundle:bundle];
    
    if (self == nil) return nil;
    
    view = [[HWMonitorView alloc] initWithFrame: [[self view] frame] menuExtra:self];
    
    [self setView:view];
    
    // Set status bar icon
    [view setImage:[[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"thermo" ofType:@"png"]]];
    [view setAlternateImage:[[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"thermotemplate" ofType:@"png"]]];
    [view setFrameSize:NSMakeSize(80, [view frame].size.height)];
    
    statusBarFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0f];
    
    menu = [[NSMenu alloc] init];
    
    [menu setAutoenablesItems: NO];
    //[menu setDelegate:(id<NSMenuDelegate>)self];
    
    NSMutableParagraphStyle * style = [[NSMutableParagraphStyle alloc] init];
    [style setLineSpacing:0];
    
    statusMenuFont = [NSFont fontWithName:@"Lucida Grande Bold" size:10.0f];
    [menu setFont:statusMenuFont];
    
    style = [[NSMutableParagraphStyle alloc] init];
    [style setTabStops:[NSArray array]];
    [style addTabStop:[[NSTextTab alloc] initWithType:NSRightTabStopType location:190.0]];

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
    
    smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    
    if ([smartReporter drives]) {
        NSArray * keys = [[smartReporter drives] allKeys];
        NSArray * values = [[smartReporter drives] allValues];
        
        for (int i = 0; i < [keys count]; i++) {
            NSATAGenericDisk * disk = [values objectAtIndex:i];
            
            if (disk && [disk isRotational]) {
                NSString * key = [keys objectAtIndex:i];
                
                [self addSensorWithKey:key andCaption:[[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] intoGroup:SMARTTemperatureSensorGroup];
            }
        }
    }
    
    [self insertFooterAndTitle:@"HARD DRIVE TEMPERATURES"];
    
    // SSD Remaining Life
    
    if ([smartReporter drives]) {
        NSArray * keys = [[smartReporter drives] allKeys];
        NSArray * values = [[smartReporter drives] allValues];
        
        for (int i = 0; i < [keys count]; i++) {
            NSATAGenericDisk * disk = [values objectAtIndex:i];
            
            if (disk && ![disk isRotational]) {
                NSString * key = [keys objectAtIndex:i];
                
                [self addSensorWithKey:key andCaption:[[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] intoGroup:SMARTRemainingLifeSensorGroup];
            }
        }
    }
    
    [self insertFooterAndTitle:@"SSD REMAINING LIFE"];
    
    // Update SMART data
    [self updateSMARTData];
    
    //Multipliers
    
    for (int i=0; i<0xA; i++)
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"MC%XC",i] andCaption:[[NSString alloc] initWithFormat:@"CPU %X",i] intoGroup:MultiplierSensorGroup];
    
    [self addSensorWithKey:@"MPkC" andCaption:@"CPU Package" intoGroup:MultiplierSensorGroup];
    
    [self insertFooterAndTitle:@"MULTIPLIERS"];
    
    // Fans
    
    for (int i=0; i<10; i++){
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
    
    if ([sensorsList count] == 0) {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:@"No sensors found or FakeSMCDevice unavailable" action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [menu insertItem:item atIndex:0];
    }
    else {
        // Main sensors timer
        NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:
                                    [self methodSignatureForSelector:@selector(updateTitlesDefault)]];
        [invocation setTarget:self];
        [invocation setSelector:@selector(updateTitlesDefault)];
        [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:2.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
        
        // Main SMART timer
        invocation = [NSInvocation invocationWithMethodSignature:
                      [self methodSignatureForSelector:@selector(updateSMARTData)]];
        [invocation setTarget:self];
        [invocation setSelector:@selector(updateSMARTData)];
        
        [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:300.0 invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
        
        [self performSelector:@selector(updateTitlesForced) withObject:nil afterDelay:0.0];
    }
    
    return self;
}

- (NSMenu *)menu
{
    return menu;
}

@end
