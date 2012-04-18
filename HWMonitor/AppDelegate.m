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

- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(NSImage*)image Sensors:(NSArray*)list;
{
    if (list && [list count] > 0) {
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
        
        [statusMenu addItem:titleItem];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[[NSUserDefaults standardUserDefaults] boolForKey:[sensor key]]];
            
            if ([sensor disk])
                [sensor setCaption:[[sensor caption] stringByTruncatingToWidth:145.0f withFont:statusMenuFont]];
            
            NSMenuItem *sensorItem = [[NSMenuItem alloc] initWithTitle:[sensor caption] action:@selector(menuItemClicked:) keyEquivalent:@""];
            
            [sensor setMenuItem:sensorItem];
            
            [sensorItem setTarget:self];
            [sensorItem setRepresentedObject:sensor];
            [sensorItem setState:[sensor favorite]];
            [sensorItem setOnStateImage:favoriteIcon];
            [sensorItem setOffStateImage:disabledIcon];
            
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
    
    NSMutableAttributedString * statusString = [[NSMutableAttributedString alloc] init];
    
    for (int i = 0; i < [sensors count]; i++) {
        HWMonitorSensor *sensor = (HWMonitorSensor*)[sensors objectAtIndex:i];
        
        NSDictionary *captionColor;
        NSDictionary *valueColor;
        
        NSString * value = [sensor formattedValue];
        
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
        
        if (isMenuVisible) {
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] init];
            
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor caption] attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:@"\t"]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:value attributes:valueColor]];
            
            [title addAttributes:statusMenuAttributes range:NSMakeRange(0, [title length])];
            [title addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [title length])];
            
            // Update menu item title
            [[sensor menuItem] setAttributedTitle:title];
        }
        
        if ([sensor favorite]) {
            NSString * value =[[NSString alloc] initWithString:[sensor formattedValue]];
            
            [statusString appendAttributedString:[[NSAttributedString alloc] initWithString:@" "]];
            
            if (!isMenuVisible && valueColor != blackColorAttribute)
                [statusString appendAttributedString:[[NSAttributedString alloc] initWithString:value attributes:valueColor]];
            else
                [statusString appendAttributedString:[[NSAttributedString alloc] initWithString:value]];
        }
    }
    
    // Update status bar title
    NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithAttributedString:statusString];
    
    [title addAttributes:statusItemAttributes range:NSMakeRange(0, [title length])];
    
    if (!isMenuVisible) 
        [title addAttribute:NSShadowAttributeName value:statusItemShadow range:NSMakeRange(0,[title length])];
         
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
    
    HWMonitorSensor *sensor = (HWMonitorSensor*)[menuItem representedObject];
    
    [sensor setFavorite:![sensor favorite]];
    
    [menuItem setState:[sensor favorite]];
    
    [self updateTitles];
    
    [[NSUserDefaults standardUserDefaults] setBool:[sensor favorite] forKey:[sensor key]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)awakeFromNib
{    
    favoriteIcon = [NSImage imageNamed:@"favorite"];
    disabledIcon = [NSImage imageNamed:@"disabled"];
    
    statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    
    [statusItem setMenu:statusMenu];
    [statusItem setHighlightMode:YES];
    [statusItem setImage:[NSImage imageNamed:@"thermobump"]];
    [statusItem setAlternateImage:[NSImage imageNamed:@"thermotemplate"]];
    
    statusItemFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0f];
    //statusItemFont = [NSFont boldSystemFontOfSize:9.0];
    //statusItemFont = [NSFont menuBarFontOfSize:9.0];

    NSMutableParagraphStyle * style = [[NSMutableParagraphStyle alloc] init];
    [style setLineSpacing:0];
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];

    [dictionary setObject:style forKey:NSParagraphStyleAttributeName];
    [dictionary setObject:statusItemFont forKey:NSFontAttributeName];
    
    statusItemAttributes = [NSDictionary dictionaryWithDictionary:dictionary];
    
    statusItemShadow = [[NSShadow alloc] init];
    
    [statusItemShadow setShadowColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.7]];
    [statusItemShadow setShadowOffset:CGSizeMake(0, -1.0)];
    [statusItemShadow setShadowBlurRadius:1.0];
    
    //statusMenuFont = [NSFont fontWithName:@"Lucida Grande Bold" size:10.0f];
    statusMenuFont = [NSFont boldSystemFontOfSize:10.0f];
    //statusMenuFont = [NSFont menuFontOfSize:10.0f];
    
    [statusMenu setFont:statusMenuFont];
    
    style = [[NSMutableParagraphStyle alloc] init];
    [style setTabStops:[NSArray array]];
    [style addTabStop:[[NSTextTab alloc] initWithType:NSRightTabStopType location:190.0]];
    [style setLineBreakMode:NSLineBreakByTruncatingTail];

    dictionary = [[NSMutableDictionary alloc] init];
    
    [dictionary setObject:style forKey:NSParagraphStyleAttributeName];
    [dictionary setObject:statusMenuFont forKey:NSFontAttributeName];
    
    statusMenuAttributes = [NSDictionary dictionaryWithDictionary:dictionary];
    
    blackColorAttribute = [NSDictionary dictionaryWithObject:[NSColor blackColor] forKey:NSForegroundColorAttributeName];
    orangeColorAttribute = [NSDictionary dictionaryWithObject:[NSColor orangeColor] forKey:NSForegroundColorAttributeName];
    redColorAttribute = [NSDictionary dictionaryWithObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
    
    // Init sensors
    monitor = [HWMonitorEngine hardwareMonitor];
    
    [monitor rebuildSensorsList];
    
    if ([[monitor sensors] count] > 0) {
        [self insertMenuGroupWithTitle:@"TEMPERATURES" Icon:[NSImage imageNamed:@"temperatures"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" Icon:[NSImage imageNamed:@"hddtemperatures"] Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" Icon:[NSImage imageNamed:@"ssdlife"] Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING BLOCKS" Icon:[NSImage imageNamed:@"ssdlife"] Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingBlocks]];
        [self insertMenuGroupWithTitle:@"MULTIPLIERS" Icon:[NSImage imageNamed:@"multipliers"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupMultiplier]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" Icon:[NSImage imageNamed:@"frequencies"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupFrequency]];
        [self insertMenuGroupWithTitle:@"FANS" Icon:[NSImage imageNamed:@"tachometers"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTachometer]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" Icon:[NSImage imageNamed:@"voltages"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
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
