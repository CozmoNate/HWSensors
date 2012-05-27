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

#include "HWMonitorView.h"
#include "HWMonitorDefinitions.h"

@implementation AppDelegate

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

- (void)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Icon:(NSImage*)image
{
    NSMenuItem *titleItem = [[NSMenuItem alloc] init];
    
    [titleItem setEnabled:FALSE];
    
    NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:GetLocalizedString(title)];
    
    [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
    [attributedTitle addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [attributedTitle length])];
    
    [titleItem setAttributedTitle:attributedTitle];
    [titleItem setImage:image];
    
    [someMenu addItem:titleItem];
}

- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(NSImage*)image Sensors:(NSArray*)list;
{
    if (list && [list count] > 0) {        
        [self insertTitleItemWithMenu:statusMenu Title:title Icon:image];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[favorites containsObject:sensor]];
            
            if ([sensor disk])
                [sensor setCaption:[[sensor caption] stringByTruncatingToWidth:145.0f withFont:statusMenuFont]];
            
            NSMenuItem *sensorItem = [[NSMenuItem alloc] initWithTitle:[sensor caption] action:@selector(sensorItemClicked:) keyEquivalent:@""];
            
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

- (NSMenuItem*)insertPrefsItemWithTitle:(NSString*)title icon:(NSImage*)image state:(NSUInteger)state action:(SEL)aSelector keyEquivalent:(NSString *)charCode
{
    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(title) action:aSelector keyEquivalent:charCode];
    
    [item setState:state];
    [item setTarget:self];
    
    [prefsMenu addItem:item];
    
    return item;
}

- (void)updateSMARTData
{
    [monitor updateSMARTSensorsValues];
}

- (void)updateSMARTDataThreaded
{
    [self performSelectorInBackground:@selector(updateSMARTData) withObject:nil];
}

- (void)updateData
{
    [monitor updateGenericSensorsValuesButOnlyFavorits:!isMenuVisible];
}

- (void)updateDataThreaded
{
    [self performSelectorInBackground:@selector(updateData) withObject:nil];
}

- (void)updateTitles
{
    [statusView setNeedsDisplay:YES];
    
    //NSMutableDictionary * favoritsList = [[NSMutableDictionary alloc] init];
    
    if (!isMenuVisible)
        return;
    
    for (int i = 0; i < [[monitor sensors] count]; i++) {
        HWMonitorSensor *sensor = [[monitor sensors] objectAtIndex:i];
        
        if (!sensor)
            continue;
        
        if ([sensor favorite] || [sensor valueHasBeenChanged]) {
            NSDictionary *captionColor;
            NSDictionary *valueColor;
            
            NSString * value = [sensor value];
            
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
            
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] init];
            
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor caption] attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:@"\t"]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:value attributes:valueColor]];
            
            [title addAttributes:statusMenuAttributes range:NSMakeRange(0, [title length])];
            [title addAttribute:NSFontAttributeName value:statusMenuFont range:NSMakeRange(0, [title length])];
            
            // Update menu item title
            [[sensor menuItem] setAttributedTitle:title];
            
            /*if ([sensor favorite]) {
                if (!isMenuVisible && valueColor != blackColorAttribute)
                    [favoritsList setObject:[[NSAttributedString alloc] initWithString:value attributes:valueColor] forKey:[sensor key]];
                else
                    [favoritsList setObject:[[NSAttributedString alloc] initWithString:value] forKey:[sensor key]];
            }*/
        }
    }
    
    // Update status bar title
    /*NSMutableAttributedString * title = [[NSMutableAttributedString alloc] init];
    
    NSUInteger i = 0;
    
    for (i = 0; i < [favorites count]; i++) {
        HWMonitorSensor *sensor = [favorites objectAtIndex:i];
        
        [title appendAttributedString:[favoritsList objectForKey:[sensor key]]];
        
        if (i < [favorites count] - 1)
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:@" "]];
    }
    
    [title addAttributes:statusItemAttributes range:NSMakeRange(0, [title length])];
    
    if (!isMenuVisible) 
        [title addAttribute:NSShadowAttributeName value:statusItemShadow range:NSMakeRange(0,[title length])];
         
    [statusItem setAttributedTitle:title];*/
}

- (void)rebuildSensors
{
    if (!monitor) {
        monitor = [[HWMonitorEngine alloc] initWithBundle:[NSBundle mainBundle]];
        [statusView setMonitor:monitor];
    }
    
    if (!favorites) {
        favorites = [[NSMutableArray alloc] init];
        [statusView setFavorites:favorites];
    }
    
    [statusMenu removeAllItems];
    
    [statusView setDrawValuesInRow:[[NSUserDefaults standardUserDefaults] boolForKey:@kHWMonitorFavoritesInRow]];
    
    [monitor setHideDisabledSensors:![[NSUserDefaults standardUserDefaults] boolForKey:@kHWMonitorShowHiddenSensors]];
    [monitor setShowBSDNames:[[NSUserDefaults standardUserDefaults] boolForKey:@kHWMonitorShowBSDNames]];
    
    [monitor rebuildSensorsList];
    
    if ([[monitor sensors] count] > 0) {
        NSUInteger i = 0;
        
        [favorites removeAllObjects];
        
        NSMutableArray *favoritsList = [[NSUserDefaults standardUserDefaults] objectForKey:@kHWMonitorFavoritesList];
        
        if (favoritsList) {
            for (i = 0; i < [favoritsList count]; i++) {
                HWMonitorSensor *sensor = [[monitor keys] objectForKey:[favoritsList objectAtIndex:i]];
                
                if (sensor) {
                    [sensor setFavorite:TRUE];
                    [favorites addObject:sensor];
                }
            }
        }
        
        [self insertMenuGroupWithTitle:@"TEMPERATURES" Icon:[NSImage imageNamed:@"temperatures"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" Icon:[NSImage imageNamed:@"hddtemperatures"] Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" Icon:[NSImage imageNamed:@"ssdlife"] Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING BLOCKS" Icon:[NSImage imageNamed:@"ssdlife"] Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingBlocks]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" Icon:[NSImage imageNamed:@"frequencies"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupFrequency | kHWSensorGroupMultiplier]];
        [self insertMenuGroupWithTitle:@"FANS" Icon:[NSImage imageNamed:@"tachometers"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTachometer | kHWSensorGroupPWM]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" Icon:[NSImage imageNamed:@"voltages"] Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
        [self updateTitles];
    }
    else {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:NSLocalizedString(@"No sensors found", nil) action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [statusMenu addItem:item];
        [statusMenu addItem:[NSMenuItem separatorItem]];
    }
    
    // Preferences...
    NSMenuItem* prefsItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(@"Preferences") action:nil keyEquivalent:@""];
    [prefsItem setImage:[NSImage imageNamed:@"preferences"]];
    [statusMenu addItem:prefsItem];
    
    prefsMenu = [[NSMenu alloc] init];
    
    [self insertTitleItemWithMenu:prefsMenu Title:@"GENERAL" Icon:nil];
    
    [self insertPrefsItemWithTitle:@"Show hidden sensors" icon:nil state:![monitor hideDisabledSensors] action:@selector(showHiddenSensorsItemClicked:) keyEquivalent:@""];
    [self insertPrefsItemWithTitle:@"Use BSD drives names" icon:nil state:[monitor showBSDNames] action:@selector(showBSDNamesItemClicked:) keyEquivalent:@""];
    [self insertPrefsItemWithTitle:@"Favorites placed in a row" icon:nil state:[statusView drawValuesInRow] action:@selector(favoritesInRowItemClicked:) keyEquivalent:@""];
    
    [prefsMenu addItem:[NSMenuItem separatorItem]];
    [self insertTitleItemWithMenu:prefsMenu Title:@"TEMPERATURE SCALE" Icon:nil];
    
    [monitor setUseFahrenheit:[[NSUserDefaults standardUserDefaults] boolForKey:@kHWMonitorUseFahrenheitKey]];
    
    celsiusItem = [self insertPrefsItemWithTitle:@"Celsius" icon:nil state:![monitor useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
    fahrenheitItem = [self insertPrefsItemWithTitle:@"Fahrenheit" icon:nil state:[monitor useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
    
    [prefsItem setSubmenu:prefsMenu];
    
    [statusMenu addItem:[NSMenuItem separatorItem]];
    
    [statusMenu addItem:[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quit HWMonitor", nil) action:@selector(terminate:) keyEquivalent:@""]];
}

// Events

- (void)menuWillOpen:(NSMenu *)menu {
    isMenuVisible = YES;
    [statusView setIsMenuDown:YES];
    
    [self updateTitles];
}

- (void)menuDidClose:(NSMenu *)menu {
    isMenuVisible = NO;
    [statusView setIsMenuDown:NO];
}

- (void)sensorItemClicked:(id)sender {
    NSMenuItem * menuItem = (NSMenuItem *)sender;
    
    HWMonitorSensor *sensor = (HWMonitorSensor*)[menuItem representedObject];
    
    if ([sensor favorite])
        [favorites removeObject:sensor];
    
    [sensor setFavorite:![sensor favorite]];
    [menuItem setState:[sensor favorite]];
    
    if ([sensor favorite])
        [favorites addObject:sensor];
    
    [self updateTitles];
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    NSUInteger i;
    
    for (i = 0; i < [favorites count]; i++) {
        sensor = [favorites objectAtIndex:i];
        [list addObject:[sensor key]];
    }
    
    [[NSUserDefaults standardUserDefaults] setObject:list forKey:@kHWMonitorFavoritesList];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)degreesItemClicked:(id)sender
{
    bool useFahrenheit = [sender isEqualTo:fahrenheitItem];
    
    if (useFahrenheit != [monitor useFahrenheit] ) {
        [celsiusItem setState:!useFahrenheit];
        [fahrenheitItem setState:useFahrenheit];
        
        [monitor setUseFahrenheit:useFahrenheit];
        
        [[NSUserDefaults standardUserDefaults] setBool:useFahrenheit forKey:@kHWMonitorUseFahrenheitKey];
        [[NSUserDefaults standardUserDefaults] synchronize];
        
        [self updateTitles];
    }
}

- (void)showHiddenSensorsItemClicked:(id)sender
{   
    [sender setState:![sender state]];
    
    [[NSUserDefaults standardUserDefaults] setBool:[sender state] forKey:@kHWMonitorShowHiddenSensors];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    [self rebuildSensors];
}

- (void)showBSDNamesItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [[NSUserDefaults standardUserDefaults] setBool:[sender state] forKey:@kHWMonitorShowBSDNames];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    [self rebuildSensors];
}

- (void)favoritesInRowItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [statusView setDrawValuesInRow:[sender state]];
    
    [[NSUserDefaults standardUserDefaults] setBool:[sender state] forKey:@kHWMonitorFavoritesInRow];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)sleepNoteReceived:(NSNotification*)note
{
    if ([note name] == NSWorkspaceWillSleepNotification) {
        
    }
}

- (void)wakeNoteReceived:(NSNotification*)note
{
    if ([note name] == NSWorkspaceDidWakeNotification) {
        [self updateSMARTDataThreaded];
        [self updateDataThreaded];
    }
}

- (void)awakeFromNib
{
    favoriteIcon = [NSImage imageNamed:@"favorite"];
    disabledIcon = [NSImage imageNamed:@"disabled"];
    
    statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    
    statusView = [[HWMonitorCustomView alloc] initWithFrame:NSMakeRect(0, 0, 24, 24) statusItem:statusItem];
    
    [statusView setImage:[NSImage imageNamed:@"thermobump"]];
    [statusView setAlternateImage:[NSImage imageNamed:@"thermotemplate"]];
    
    [statusItem setView:statusView];
    [statusItem setMenu:statusMenu];
    [statusItem setHighlightMode:YES];
    
    statusItemFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0];
    //statusItemFont = [NSFont boldSystemFontOfSize:9.0];
    //statusItemFont = [NSFont menuBarFontOfSize:9.0];

    NSMutableParagraphStyle * style = [[NSMutableParagraphStyle alloc] init];
    [style setLineSpacing:0];
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];

    [dictionary setObject:style forKey:NSParagraphStyleAttributeName];
    [dictionary setObject:statusItemFont forKey:NSFontAttributeName];
    
    statusItemAttributes = [NSDictionary dictionaryWithDictionary:dictionary];
    
    statusItemShadow = [[NSShadow alloc] init];
    
    [statusItemShadow setShadowColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.55]];
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
    [self rebuildSensors];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Register PM events
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(sleepNoteReceived:) name: NSWorkspaceWillSleepNotification object: NULL];
    
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(wakeNoteReceived:) name: NSWorkspaceDidWakeNotification object: NULL];

    NSInvocation *invocation = nil;

    // Main SMART timer
    invocation = [NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(updateSMARTDataThreaded)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateSMARTDataThreaded)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:300.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    // Main sensors timer
    invocation = [NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(updateDataThreaded)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateDataThreaded)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:2.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    // Update menu titles timer
    invocation = [NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(updateTitles)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateTitles)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:0.5f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    [self updateTitles];
}

@end
