//
//  AppDelegate.m
//  HWMonitor
//
//  Created by mozo on 20.10.11.
//  Copyright (c) 2011 mozo. All rights reserved.
//

#import "AppDelegate.h"
#import "NSString+TruncateToWidth.h"

#include "FakeSMCDefinitions.h"

@implementation AppDelegate

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

- (void)insertMenuGroupWithTitle:(NSString*)title  sensors:(NSArray*)list;
{
    if (list && [list count] > 0) {
        NSMenuItem *titleItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(title) action:nil keyEquivalent:@""];
        
        [titleItem setEnabled:FALSE];
        
        [statusMenu addItem:titleItem];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[[NSUserDefaults standardUserDefaults] boolForKey:[sensor key]]];
            
            if ([sensor disk])
                [sensor setCaption:[[sensor caption] stringByTruncatingToWidth:130.0f withFont:statusItemFont]];
            
            NSMenuItem *sensorItem = [[NSMenuItem alloc] initWithTitle:[sensor caption] action:@selector(menuItemClicked:) keyEquivalent:@""];
            
            [sensor setMenuItem:sensorItem];
            
            [sensorItem setTarget:self];
            [sensorItem setRepresentedObject:sensor];
            
            if ([sensor favorite]) [sensorItem setState:TRUE]; 
            
            [statusMenu addItem:sensorItem];
        }
        
        [statusMenu addItem:[NSMenuItem separatorItem]];
    }
}

- (void)updateSMARTData; 
{
    [monitor updateSMARTSensorsValues];
}

- (void)updateTitles
{
    [monitor updateGenericSensorsValuesButOnlyFavorits:!isMenuVisible];
    
    NSArray *sensors = [monitor sensors];
    
    NSMutableString * statusString = [[NSMutableString alloc] init];
    
    for (int i = 0; i < [sensors count]; i++) {
        HWMonitorSensor *sensor = (HWMonitorSensor*)[sensors objectAtIndex:i];
        
        if (isMenuVisible) {
            NSString * value = [sensor formatValue];
            
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:[[NSString alloc] initWithFormat:@"%S\t%S",[[sensor caption] cStringUsingEncoding:NSUTF16StringEncoding],[value cStringUsingEncoding:NSUTF16StringEncoding]] attributes:statusMenuAttributes];
            
            [title addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [title length])];
            
            // Update menu item title
            [[sensor menuItem] setAttributedTitle:title];
        }
        
        if ([sensor favorite]) {
            NSString * value =[[NSString alloc] initWithString:[sensor formatValue]];
            
            [statusString appendString:@" "];
            [statusString appendString:value];
        }
    }
    
    // Update status bar title
    NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:statusString attributes:statusItemAttributes];
    
    [title addAttribute:NSFontAttributeName value:statusItemFont range:NSMakeRange(0, [title length])];
    
    [statusItem setAttributedTitle:title];
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
    
    HWMonitorSensor *sensor = (HWMonitorSensor*)[menuItem representedObject];
    
    [sensor setFavorite:[menuItem state]];
    
    [self updateTitles];
    
    [[NSUserDefaults standardUserDefaults] setBool:[menuItem state] forKey:[sensor key]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)awakeFromNib
{
    statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    
    [statusItem setMenu:statusMenu];
    [statusItem setHighlightMode:YES];
    [statusItem setImage:[NSImage imageNamed:@"thermobump"]];
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
    monitor = [HWMonitorEngine hardwareMonitor];
    
    [monitor rebuildSensorsList];
    
    if ([[monitor sensors] count] > 0) {
        [self insertMenuGroupWithTitle:@"TEMPERATURES" sensors:[monitor getAllSensorsInGroup:kHWTemperatureGroup]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" sensors:[monitor getAllSensorsInGroup:kHWSMARTTemperatureGroup]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" sensors:[monitor getAllSensorsInGroup:kHWSMARTRemainingLifeGroup]];
        [self insertMenuGroupWithTitle:@"MULTIPLIERS" sensors:[monitor getAllSensorsInGroup:kHWMultiplierGroup]];
        [self insertMenuGroupWithTitle:@"FANS" sensors:[monitor getAllSensorsInGroup:kHWTachometerGroup]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" sensors:[monitor getAllSensorsInGroup:kHWVoltageGroup]];
        
        [self updateTitles];
    }
    else {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:NSLocalizedString(@"No sensors found", nil) action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [statusMenu addItem:item];
        [statusMenu addItem:[NSMenuItem separatorItem]];
    }
    
    [statusMenu addItem:[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quit HWMonitor", nil) action:@selector(terminate:) keyEquivalent:@""]];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Main sensors timer
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:
                                [self methodSignatureForSelector:@selector(updateTitles)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateTitles)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:2.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    // Main SMART timer
    invocation = [NSInvocation invocationWithMethodSignature:
                  [self methodSignatureForSelector:@selector(updateSMARTData)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateSMARTData)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:300.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    [self updateTitles];
}

@end
