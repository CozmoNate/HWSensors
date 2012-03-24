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
        
        NSMenuItem *titleItem = [[NSMenuItem alloc] init];
        
        [titleItem setEnabled:FALSE];
        
        NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:GetLocalizedString(title)];
        
        [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
        [attributedTitle addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [attributedTitle length])];
        
        [titleItem setAttributedTitle:attributedTitle];
        
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
            [sensorItem setState:[sensor favorite]];
            [sensorItem setOnStateImage:stateGem];
            
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
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] init];
            
            NSDictionary *captionColor;
            NSDictionary *valueColor;
            
            switch ([sensor level]) {
                    /*case kHWSensorLevelDisabled:
                     break;
                     
                     case kHWSensorLevelNormal:
                     break;*/
                    
                case kHWSensorLevelModerate:
                    captionColor = blackColorAttribute;
                    valueColor = orangeColorAttribute;
                    break;
                    
                case kHWSensorLevelHigh:
                    captionColor = blackColorAttribute;
                    valueColor = redColorAttribute;
                    break;
                    
                case kHWSensorLevelExceeded:
                    captionColor = redColorAttribute;
                    valueColor = redColorAttribute;
                    break;
                    
                default:
                    captionColor = blackColorAttribute;
                    valueColor = blackColorAttribute;
                    break;
            }
            
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor caption] attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:@"\t" attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor formatValue] attributes:valueColor]];
            
            [title addAttributes:statusMenuAttributes range:NSMakeRange(0, [title length])];
            
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

- (void)rebuildSensors
{
    if (!monitor)
        monitor = [[HWMonitorEngine alloc] initWithBundle:[self bundle]];
    
    [[self menu] removeAllItems];
    
    [monitor rebuildSensorsList];
    
    if ([[monitor sensors] count] > 0) {
        [self insertMenuGroupWithTitle:@"TEMPERATURES" sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"MULTIPLIERS" sensors:[monitor getAllSensorsInGroup:kHWSensorGroupMultiplier]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" sensors:[monitor getAllSensorsInGroup:kHWSensorGroupFrequency]];
        [self insertMenuGroupWithTitle:@"FANS" sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTachometer]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" sensors:[monitor getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
        /*[menu addItem:[NSMenuItem separatorItem]];
        [menu addItem:[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Refresh sensors", nil) action:@selector(rebuildSensors:) keyEquivalent:@""]];*/
        
        [self updateTitlesForced];
    }
    else {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:NSLocalizedString(@"No sensors found", nil) action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [menu addItem:item];
    }
}

- (void)menuItemClicked:(id)sender 
{
    NSMenuItem * menuItem = (NSMenuItem *)sender;
    HWMonitorSensor *sensor = (HWMonitorSensor*)[menuItem representedObject];
    
    [sensor setFavorite:![sensor favorite]];
    
    [menuItem setState:[sensor favorite]];
    
    [self updateTitlesDefault];
    
    [[NSUserDefaults standardUserDefaults] setBool:[sensor favorite] forKey:[sensor key]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (id)initWithBundle:(NSBundle *)bundle
{
    self = [super initWithBundle:bundle];
    
    if (self == nil) return nil;
    
    stateGem = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"favorite" ofType:@"png"]];
    
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
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
    
    [dictionary setObject:style forKey:NSParagraphStyleAttributeName];
    [dictionary setObject:statusMenuFont forKey:NSFontAttributeName];
    
    statusMenuAttributes = [NSDictionary dictionaryWithDictionary:dictionary];
    
    blackColorAttribute = [NSDictionary dictionaryWithObject:[NSColor blackColor] forKey:NSForegroundColorAttributeName];
    orangeColorAttribute = [NSDictionary dictionaryWithObject:[NSColor orangeColor] forKey:NSForegroundColorAttributeName];
    redColorAttribute = [NSDictionary dictionaryWithObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
    
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
    
    // Rebuild sensors timer
    invocation = [NSInvocation invocationWithMethodSignature:
                  [self methodSignatureForSelector:@selector(rebuildSensors)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(rebuildSensors)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:1800.0 invocation:invocation repeats:YES] forMode:NSDefaultRunLoopMode];
    
    [self performSelector:@selector(rebuildSensors) withObject:nil afterDelay:0.5];
    
    return self;
}

- (NSMenu *)menu
{
    return menu;
}

@end
