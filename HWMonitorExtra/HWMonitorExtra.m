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

#define GetLocalizedString(key) \
[[self bundle] localizedStringForKey:(key) value:@"" table:nil]

- (void)insertMenuGroupWithTitle:(NSString*)title  sensors:(NSArray*)list;
{
    if (list && [list count] > 0) {
        if ([[menu itemArray] count] > 0)
            [menu addItem:[NSMenuItem separatorItem]];
        
        NSMenuItem *titleItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(title) action:nil keyEquivalent:@""];
        
        [titleItem setEnabled:FALSE];
        
        [menu addItem:titleItem];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[[NSUserDefaults standardUserDefaults] boolForKey:[sensor key]]];
            
            if ([sensor disk])
                [sensor setCaption:[[sensor caption] stringByTruncatingToWidth:130.0f withFont:statusBarFont]];
            
            NSMenuItem * sensorItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString((NSString*)[sensor caption]) action:@selector(menuItemClicked:) keyEquivalent:@""];
            
            [sensor setMenuItem:sensorItem];
            
            [sensorItem setTarget:self];
            [sensorItem setRepresentedObject:sensor];
            
            if ([sensor favorite]) [sensorItem setState:TRUE]; 
            
            [menu addItem:sensorItem];
        }
    }
}

- (void)updateSMARTData; 
{
    [monitor updateSMARTSensorsValues];
}

- (void)updateTitlesForceAllSensors:(BOOL)allSensors
{
    [monitor updateGenericSensorsValuesButOnlyFavorits:!allSensors && ![self isMenuDown]];
    
    NSMutableArray * favorites = [[NSMutableArray alloc] init];
    
    NSArray *sensors = [monitor sensors];
    
    for (int i = 0; i < [sensors count]; i++) {
        HWMonitorSensor *sensor = (HWMonitorSensor*)[sensors objectAtIndex:i];
        
        if ([self isMenuDown] || allSensors) {
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:[[NSString alloc] initWithFormat:@"%S\t%S",[[sensor caption] cStringUsingEncoding:NSUTF16StringEncoding],[[sensor formatValue] cStringUsingEncoding:NSUTF16StringEncoding]] attributes:statusMenuAttributes];
            
            [title addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [title length])];
            
            // Update menu item title
            [[sensor menuItem] setAttributedTitle:title];
        }
        
        if ([sensor favorite])
            [favorites addObject:[[NSString alloc] initWithFormat:@"%S", [[sensor formatValue] cStringUsingEncoding:NSUTF16StringEncoding]]];
    }
    
    // Update status bar title
    if ([favorites count] > 0)
        [view setTitles:favorites];
    else 
        [view setTitles:nil];
}

- (void)updateTitlesForced
{
    [self updateTitlesForceAllSensors:YES];
}

- (void)updateTitlesDefault
{
    [self updateTitlesForceAllSensors:NO];
}

- (void)menuItemClicked:(id)sender 
{
    NSMenuItem * menuItem = (NSMenuItem *)sender;
    
    [menuItem setState:![menuItem state]];
    
    HWMonitorSensor *sensor = (HWMonitorSensor*)[menuItem representedObject];
    
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
    [view setImage:[[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"thermobump" ofType:@"png"]]];
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
    
    monitor = [[HWMonitorEngine alloc] initWithBundle:bundle];
    
    [monitor rebuildSensorsList];
    
    if ([[monitor sensors] count] > 0) {
        [self insertMenuGroupWithTitle:@"TEMPERATURES" sensors:[monitor getAllSensorsInGroup:kHWTemperatureGroup]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" sensors:[monitor getAllSensorsInGroup:kHWSMARTTemperatureGroup]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" sensors:[monitor getAllSensorsInGroup:kHWSMARTRemainingLifeGroup]];
        [self insertMenuGroupWithTitle:@"MULTIPLIERS" sensors:[monitor getAllSensorsInGroup:kHWMultiplierGroup]];
        [self insertMenuGroupWithTitle:@"FANS" sensors:[monitor getAllSensorsInGroup:kHWTachometerGroup]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" sensors:[monitor getAllSensorsInGroup:kHWVoltageGroup]];
        
        [self updateTitlesForced];
        
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
    else {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:NSLocalizedString(@"No sensors found", nil) action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [menu addItem:item];
    }
    
    return self;
}

- (NSMenu *)menu
{
    return menu;
}

@end
