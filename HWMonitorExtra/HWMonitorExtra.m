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

#include "FakeSMCDefinitions.h"

@implementation HWMonitorExtra

#define GetLocalizedString(key) \
[[self bundle] localizedStringForKey:(key) value:@"" table:nil]

- (NSMenu *)menu
{
    return menu;
}

- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(NSImage*)image Sensors:(NSArray*)list;
{
    if (list && [list count] > 0) {
        if ([[menu itemArray] count] > 0)
            [menu addItem:[NSMenuItem separatorItem]];
        
        NSMenuItem *titleItem = [[NSMenuItem alloc] init];
        
        [titleItem setEnabled:FALSE];
        
        //NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:[NSString localizedStringWithFormat:@" %@", GetLocalizedString(title)]];
        NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:GetLocalizedString(title)];
        
        [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
        [attributedTitle addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [attributedTitle length])];
        
        [titleItem setAttributedTitle:attributedTitle];
        [titleItem setImage:image];
        //[titleItem setOnStateImage:image];
        //[titleItem setState:YES];
        
        [menu addItem:titleItem];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[defaults boolForKey:[sensor key]]];
            
            if ([sensor disk])
                [sensor setCaption:[[sensor caption] stringByTruncatingToWidth:145.0f withFont:statusMenuFont]];
            
            NSMenuItem * sensorItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString((NSString*)[sensor caption]) action:@selector(menuItemClicked:) keyEquivalent:@""];
            
            [sensor setMenuItem:sensorItem];
            
            [sensorItem setTarget:self];
            [sensorItem setRepresentedObject:sensor];
            [sensorItem setState:[sensor favorite]];
            [sensorItem setOnStateImage:favoriteIcon];
            [sensorItem setOffStateImage:disabledIcon];
            
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
    
    [view setNeedsDisplay:YES];
    
    if (!allSensors && ![self isMenuDown])
        return;
    
    NSArray *sensors = [monitor sensors];
    
    for (int i = 0; i < [sensors count]; i++) {
        HWMonitorSensor *sensor = (HWMonitorSensor*)[sensors objectAtIndex:i];
        
        if (sensor && ([self isMenuDown] || allSensors) && [sensor valueHasBeenChanged]) {
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
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor value] attributes:valueColor]];
            
            [title addAttributes:statusMenuAttributes range:NSMakeRange(0, [title length])];
            
            // Update menu item title
            [[sensor menuItem] setAttributedTitle:title];
        }
    }
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
    
    [view setMonitor:monitor];
    
    if ([[monitor sensors] count] > 0) {
        [self insertMenuGroupWithTitle:@"TEMPERATURES" Icon:temperaturesIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" Icon:hddtemperaturesIcon Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" Icon:ssdlifeIcon Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING BLOCKS" Icon:ssdlifeIcon Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingBlocks]];
        //[self insertMenuGroupWithTitle:@"MULTIPLIERS" Icon:multipliersIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupMultiplier]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" Icon:frequenciesIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupFrequency]];
        [self insertMenuGroupWithTitle:@"FANS" Icon:tachometersIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTachometer]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" Icon:voltagesIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
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
    
    [defaults setBool:[sensor favorite] forKey:[sensor key]];
    [defaults synchronize];
}

- (id)initWithBundle:(NSBundle *)bundle
{
    self = [super initWithBundle:bundle];
    
    if (self == nil) return nil;
    
    defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.kozlek.HWMonitor"];
    
    favoriteIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"favorite" ofType:@"png"]];
    disabledIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"disabled" ofType:@"png"]];
    temperaturesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"temperatures" ofType:@"png"]];
    hddtemperaturesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"hddtemperatures" ofType:@"png"]];
    ssdlifeIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"ssdlife" ofType:@"png"]];
    multipliersIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"multipliers" ofType:@"png"]];
    frequenciesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"frequencies" ofType:@"png"]];
    tachometersIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"tachometers" ofType:@"png"]];
    voltagesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"voltages" ofType:@"png"]];
    
    view = [[HWMonitorView alloc] initWithFrame: [[self view] frame] menuExtra:self];
    
    [self setView:view];
    
    // Set status bar icon
    [view setImage:[[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"thermobump" ofType:@"png"]]];
    [view setAlternateImage:[[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"thermotemplate" ofType:@"png"]]];
    [view setFrameSize:NSMakeSize(80, [view frame].size.height)];
    
    menu = [[NSMenu alloc] init];
    
    [menu setAutoenablesItems: NO];
    //[menu setDelegate:(id<NSMenuDelegate>)self];
    
    NSMutableParagraphStyle * style = [[NSMutableParagraphStyle alloc] init];
    [style setLineSpacing:0];
    
    //statusMenuFont = [NSFont fontWithName:@"Lucida Grande Bold" size:10.0f];
    statusMenuFont = [NSFont boldSystemFontOfSize:10.0];
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

@end
