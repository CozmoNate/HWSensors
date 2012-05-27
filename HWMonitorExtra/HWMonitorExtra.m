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
#include "HWMonitorDefinitions.h"

@implementation HWMonitorExtra

#define GetLocalizedString(key) \
[[self bundle] localizedStringForKey:(key) value:@"" table:nil]

- (NSMenu *)menu
{
    return menu;
}

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
        if ([[menu itemArray] count] > 0)
            [menu addItem:[NSMenuItem separatorItem]];
        
        [self insertTitleItemWithMenu:menu Title:title Icon:image];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[favorites containsObject:sensor]];
            
            if ([sensor disk])
                [sensor setCaption:[[sensor caption] stringByTruncatingToWidth:145.0f withFont:statusMenuFont]];
            
            NSMenuItem * sensorItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString((NSString*)[sensor caption]) action:@selector(sensorItemClicked:) keyEquivalent:@""];
            
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

- (NSMenuItem*)insertPrefsItemWithTitle:(NSString*)title icon:(NSImage*)image state:(NSUInteger)state action:(SEL)aSelector keyEquivalent:(NSString *)charCode
{
    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(title) action:aSelector keyEquivalent:charCode];
    
    [item setState:state];
    [item setTarget:self];
    
    [prefsMenu addItem:item];
    
    return item;
}

- (void)updateSMARTData; 
{
    [monitor updateSMARTSensorsValues];
}

- (void)updateSMARTDataThreaded
{
    [self performSelectorInBackground:@selector(updateSMARTData) withObject:nil];
}

- (void)updateData
{
    [monitor updateGenericSensorsValuesButOnlyFavorits:![self isMenuDown]];
}

- (void)updateDataThreaded
{
    [self performSelectorInBackground:@selector(updateData) withObject:nil];
}

- (void)updateTitlesForceAllSensors:(BOOL)allSensors
{    
    [view setNeedsDisplay:YES];
    
    if (!allSensors && ![self isMenuDown]) 
        return;
    
    //NSMutableDictionary * favoritsList = [[NSMutableDictionary alloc] init];
    
    for (int i = 0; i < [[monitor sensors] count]; i++) {
        HWMonitorSensor *sensor = [[monitor sensors] objectAtIndex:i];
        
        if (sensor && ([self isMenuDown] || allSensors) && [sensor valueHasBeenChanged]) {
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] init];
            
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
            
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor caption] attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:@"\t" attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:value attributes:valueColor]];
            
            [title addAttributes:statusMenuAttributes range:NSMakeRange(0, [title length])];
            
            // Update menu item title
            [[sensor menuItem] setAttributedTitle:title];
            
            /*if ([sensor favorite]) {
                if (![self isMenuDown] && valueColor != blackColorAttribute)
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
     
     [self setAttributedTitle:title];*/
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
    if (!monitor) {
        monitor = [[HWMonitorEngine alloc] initWithBundle:[self bundle]];
        [view setMonitor:monitor];
    }
    
    if (!favorites) {
        favorites = [[NSMutableArray alloc] init];
        [view setFavorites:favorites];
    }
    
    [[self menu] removeAllItems];
    
    [view setDrawValuesInRow:[defaults boolForKey:@kHWMonitorFavoritesInRow]];
    
    [monitor setHideDisabledSensors:![defaults boolForKey:@kHWMonitorShowHiddenSensors]];
    [monitor setShowBSDNames:[defaults boolForKey:@kHWMonitorShowBSDNames]];
    
    [monitor rebuildSensorsList];
    
    if ([[monitor sensors] count] > 0) {

        NSUInteger i = 0;
        
        [favorites removeAllObjects];
        
        NSMutableArray *favoritsList = [defaults objectForKey:@kHWMonitorFavoritesList];
        
        if (favoritsList) {
            for (i = 0; i < [favoritsList count]; i++) {
                HWMonitorSensor *sensor = [[monitor keys] objectForKey:[favoritsList objectAtIndex:i]];
                
                if (sensor) {
                    [sensor setFavorite:TRUE];
                    [favorites addObject:sensor];
                }
            }
        }
        
        [self insertMenuGroupWithTitle:@"TEMPERATURES" Icon:temperaturesIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" Icon:hddtemperaturesIcon Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" Icon:ssdlifeIcon Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING BLOCKS" Icon:ssdlifeIcon Sensors:[monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingBlocks]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" Icon:frequenciesIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency]];
        [self insertMenuGroupWithTitle:@"FANS" Icon:tachometersIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" Icon:voltagesIcon Sensors:[monitor getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
        [menu addItem:[NSMenuItem separatorItem]];
        
        // Preferences...
        NSMenuItem* prefsItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(@"Preferences") action:nil keyEquivalent:@""];
        [prefsItem setImage:prefsIcon];
        [menu addItem:prefsItem];
        
        prefsMenu = [[NSMenu alloc] init];
        
        [self insertTitleItemWithMenu:prefsMenu Title:@"GENERAL" Icon:nil];
        
        [self insertPrefsItemWithTitle:@"Show hidden sensors" icon:nil state:![monitor hideDisabledSensors] action:@selector(showHiddenSensorsItemClicked:) keyEquivalent:@""];
        [self insertPrefsItemWithTitle:@"Use BSD drives names" icon:nil state:[monitor showBSDNames] action:@selector(showBSDNamesItemClicked:) keyEquivalent:@""];
        [self insertPrefsItemWithTitle:@"Favorites placed in a row" icon:nil state:[view drawValuesInRow] action:@selector(favoritesInRowItemClicked:) keyEquivalent:@""];
        
        [prefsMenu addItem:[NSMenuItem separatorItem]];
        
        [self insertTitleItemWithMenu:prefsMenu Title:@"TEMPERATURE SCALE" Icon:nil];
        
        [monitor setUseFahrenheit:[defaults boolForKey:@kHWMonitorUseFahrenheitKey]];
        
        celsiusItem = [self insertPrefsItemWithTitle:@"Celsius" icon:nil state:![monitor useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
        fahrenheitItem = [self insertPrefsItemWithTitle:@"Fahrenheit" icon:nil state:[monitor useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
        
        [prefsItem setSubmenu:prefsMenu];
        
        [self updateTitlesForced];
    }
    else {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:NSLocalizedString(@"No sensors found", nil) action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [menu addItem:item];
    }

}

- (void)sensorItemClicked:(id)sender 
{
    NSMenuItem * menuItem = (NSMenuItem *)sender;
    HWMonitorSensor *sensor = (HWMonitorSensor*)[menuItem representedObject];
    
    if ([sensor favorite])
        [favorites removeObject:sensor];
    
    [sensor setFavorite:![sensor favorite]];
    [menuItem setState:[sensor favorite]];
    
    if ([sensor favorite])
        [favorites addObject:sensor];
    
    [self updateTitlesDefault];
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    NSUInteger i;
    
    for (i = 0; i < [favorites count]; i++) {
        sensor = [favorites objectAtIndex:i];
        [list addObject:[sensor key]];
    }
    
    [defaults setObject:list forKey:@kHWMonitorFavoritesList];
    [defaults synchronize];
}

- (void)degreesItemClicked:(id)sender
{
    bool useFahrenheit = [sender isEqualTo:fahrenheitItem];
    
    if (useFahrenheit != [monitor useFahrenheit] ) {
        [celsiusItem setState:!useFahrenheit];
        [fahrenheitItem setState:useFahrenheit];
     
        [monitor setUseFahrenheit:useFahrenheit];
        
        [defaults setBool:useFahrenheit forKey:@kHWMonitorUseFahrenheitKey];
        [defaults synchronize];
        
        [self updateTitlesForced];
    }
}

- (void)showHiddenSensorsItemClicked:(id)sender
{   
    [sender setState:![sender state]];
    
    [defaults setBool:[sender state] forKey:@kHWMonitorShowHiddenSensors];
    [defaults synchronize];
    
    [self rebuildSensors];
}

- (void)showBSDNamesItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [defaults setBool:[sender state] forKey:@kHWMonitorShowBSDNames];
    [defaults synchronize];
    
    [self rebuildSensors];
}

- (void)favoritesInRowItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [view setDrawValuesInRow:[sender state]];
    
    [defaults setBool:[sender state] forKey:@kHWMonitorFavoritesInRow];
    [defaults synchronize];
}

- (void)sleepNoteReceived:(NSNotification*)note
{
    //NSLog(@"receiveSleepNote: %@", [note name]);
    
    if ([note name] == NSWorkspaceWillSleepNotification) {
        
    }
}

- (void)wakeNoteReceived:(NSNotification*)note
{
    //NSLog(@"receiveSleepNote: %@", [note name]);
    
    if ([note name] == NSWorkspaceDidWakeNotification) {
        [self updateSMARTDataThreaded];
        [self updateDataThreaded];
    }
}

- (id)initWithBundle:(NSBundle *)bundle
{
    self = [super initWithBundle:bundle];
    
    if (self == nil) return nil;
    
    defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];
    
    favoriteIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"favorite" ofType:@"png"]];
    disabledIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"disabled" ofType:@"png"]];
    temperaturesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"temperatures" ofType:@"png"]];
    hddtemperaturesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"hddtemperatures" ofType:@"png"]];
    ssdlifeIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"ssdlife" ofType:@"png"]];
    multipliersIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"multipliers" ofType:@"png"]];
    frequenciesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"frequencies" ofType:@"png"]];
    tachometersIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"tachometers" ofType:@"png"]];
    voltagesIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"voltages" ofType:@"png"]];
    prefsIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"preferences" ofType:@"png"]];
    
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
    
    NSInvocation *invocation = nil;
    
    // Main sensors timer
    invocation = [NSInvocation invocationWithMethodSignature:
                                [self methodSignatureForSelector:@selector(updateDataThreaded)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateDataThreaded)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:2.0f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    // Main SMART timer
    invocation = [NSInvocation invocationWithMethodSignature:
                  [self methodSignatureForSelector:@selector(updateSMARTDataThreaded)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateSMARTDataThreaded)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:300.0 invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    // Update titles timer
    invocation = [NSInvocation invocationWithMethodSignature:
                                [self methodSignatureForSelector:@selector(updateTitlesDefault)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateTitlesDefault)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:0.5f invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    // Rebuild sensors timer
    invocation = [NSInvocation invocationWithMethodSignature:
                  [self methodSignatureForSelector:@selector(rebuildSensors)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(rebuildSensors)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:1800.0 invocation:invocation repeats:YES] forMode:NSDefaultRunLoopMode];
    
    [self performSelector:@selector(rebuildSensors) withObject:nil afterDelay:0.5];
    
    // Register PM events
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(sleepNoteReceived:) name: NSWorkspaceWillSleepNotification object: NULL];
    
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(wakeNoteReceived:) name: NSWorkspaceDidWakeNotification object: NULL];
    
    return self;
}

@end
