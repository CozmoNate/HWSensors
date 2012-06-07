//
//  HWMonitor.m
//  HWSensors
//
//  Created by kozlek on 06.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitor.h"
#import "HWMonitorDefinitions.h"

#import "FakeSMCDefinitions.h"

#import "NSString+TruncateToWidth.h"

#define GetLocalizedString(key) \
[_bundle localizedStringForKey:(key) value:@"" table:nil]

@implementation HWMonitor

@synthesize menu=_mainMenu;

- (id)initWithStatusItem:(NSStatusItem*)item bundle:(NSBundle*)bundle;
{
    _statusItem = item;
    _bundle = bundle;
    
    if (_bundle == nil || _statusItem == nil) 
        return nil;
        
    _isMenuExtra = [_statusItem respondsToSelector:@selector(drawMenuBackground:)];
    
    _defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];
    
    _favoriteIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"favorite" ofType:@"png"]];
    _disabledIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"disabled" ofType:@"png"]];
    _temperaturesIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"temperatures" ofType:@"png"]];
    _hddtemperaturesIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"hddtemperatures" ofType:@"png"]];
    _ssdlifeIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"ssdlife" ofType:@"png"]];
    _multipliersIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"multipliers" ofType:@"png"]];
    _frequenciesIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"frequencies" ofType:@"png"]];
    _tachometersIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"tachometers" ofType:@"png"]];
    _voltagesIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"voltages" ofType:@"png"]];
    _prefsIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"preferences" ofType:@"png"]];
    
    _view = [[HWMonitorView alloc] initWithFrame:NSMakeRect(0, 0, 24, 22) statusItem:_statusItem];
    [_view setImage:[[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"thermobump" ofType:@"png"]]];
    [_view setAlternateImage:[[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"thermotemplate" ofType:@"png"]]];
    [_view setUseShadowEffect:YES];
    
    _mainMenu = [[NSMenu alloc] init];
    
    [_mainMenu setAutoenablesItems: NO];
    [_mainMenu setDelegate:(id<NSMenuDelegate>)_view];
    
    _menuFont = [NSFont boldSystemFontOfSize:10.0];
    [_mainMenu setFont:_menuFont];
    
    NSMutableParagraphStyle* style = [[NSMutableParagraphStyle alloc] init];
    [style setTabStops:[NSArray array]];
    [style addTabStop:[[NSTextTab alloc] initWithType:NSRightTabStopType location:190.0]];
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
    
    [dictionary setObject:style forKey:NSParagraphStyleAttributeName];
    [dictionary setObject:_menuFont forKey:NSFontAttributeName];
    
    _menuAttributes = [NSDictionary dictionaryWithDictionary:dictionary];
    
    _blackColorAttribute = [NSDictionary dictionaryWithObject:[NSColor blackColor] forKey:NSForegroundColorAttributeName];
    _orangeColorAttribute = [NSDictionary dictionaryWithObject:[NSColor orangeColor] forKey:NSForegroundColorAttributeName];
    _redColorAttribute = [NSDictionary dictionaryWithObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
    
    [_statusItem setMenu:_mainMenu];
    [_statusItem setView:_view];
    
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
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(systemWillSleep:) name: NSWorkspaceWillSleepNotification object: NULL];
    
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(systemDidWake:) name: NSWorkspaceDidWakeNotification object: NULL];
    
    return self;
}

- (void)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Icon:(NSImage*)image
{
    NSMenuItem *titleItem = [[NSMenuItem alloc] init];
    
    [titleItem setEnabled:FALSE];
    
    NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:GetLocalizedString(title)];
    
    [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
    [attributedTitle addAttribute:NSFontAttributeName value:_menuFont range:NSMakeRange(0, [attributedTitle length])];
    
    [titleItem setAttributedTitle:attributedTitle];
    [titleItem setImage:image];
    
    [someMenu addItem:titleItem];
}

- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(NSImage*)image Sensors:(NSArray*)list;
{
    if (list && [list count] > 0) {
        if ([[_mainMenu itemArray] count] > 0)
            [_mainMenu addItem:[NSMenuItem separatorItem]];
        
        [self insertTitleItemWithMenu:_mainMenu Title:title Icon:image];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[_favorites containsObject:sensor]];
            
            if ([sensor disk])
                [sensor setCaption:[[sensor caption] stringByTruncatingToWidth:145.0f withFont:_menuFont]];
            
            NSMenuItem * sensorItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString((NSString*)[sensor caption]) action:@selector(sensorItemClicked:) keyEquivalent:@""];
            
            [sensor setMenuItem:sensorItem];
            
            [sensorItem setTarget:self];
            [sensorItem setRepresentedObject:sensor];
            [sensorItem setState:[sensor favorite]];
            [sensorItem setOnStateImage:_favoriteIcon];
            [sensorItem setOffStateImage:_disabledIcon];
            
            [_mainMenu addItem:sensorItem];
        }
    }
}

- (NSMenuItem*)insertPrefsItemWithTitle:(NSString*)title icon:(NSImage*)image state:(NSUInteger)state action:(SEL)aSelector keyEquivalent:(NSString *)charCode
{
    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(title) action:aSelector keyEquivalent:charCode];
    
    [item setState:state];
    [item setTarget:self];
    
    [_prefsMenu addItem:item];
    
    return item;
}

- (void)updateSMARTData; 
{
    [_monitor updateSMARTSensorsValues];
}

- (void)updateSMARTDataThreaded
{
    [self performSelectorInBackground:@selector(updateSMARTData) withObject:nil];
}

- (void)updateData
{
    [_monitor updateGenericSensorsValuesButOnlyFavorits:![_view isMenuDown]];
}

- (void)updateDataThreaded
{
    [self performSelectorInBackground:@selector(updateData) withObject:nil];
}

- (void)updateTitlesForceAllSensors:(BOOL)allSensors
{    
    [_view setNeedsDisplay:YES];
    
    if (!allSensors && ![_view isMenuDown]) 
        return;
    
    //NSMutableDictionary * favoritsList = [[NSMutableDictionary alloc] init];
    
    for (int i = 0; i < [[_monitor sensors] count]; i++) {
        HWMonitorSensor *sensor = [[_monitor sensors] objectAtIndex:i];
        
        if (sensor && ([_view isMenuDown] || allSensors) && [sensor valueHasBeenChanged]) {
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
                    captionColor = _blackColorAttribute;
                    valueColor = _orangeColorAttribute;
                    break;
                    
                case kHWSensorLevelHigh:
                    captionColor = _blackColorAttribute;
                    valueColor = _redColorAttribute;
                    break;
                    
                case kHWSensorLevelExceeded:
                    captionColor = _redColorAttribute;
                    valueColor = _redColorAttribute;
                    break;
                    
                default:
                    captionColor = _blackColorAttribute;
                    valueColor = _blackColorAttribute;
                    break;
            }
            
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor caption] attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:@"\t" attributes:captionColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:value attributes:valueColor]];
            
            [title addAttributes:_menuAttributes range:NSMakeRange(0, [title length])];
            
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
    if (!_monitor) {
        _monitor = [[HWMonitorEngine alloc] initWithBundle:_bundle];
        [_view setMonitor:_monitor];
    }
    
    if (!_favorites) {
        _favorites = [[NSMutableArray alloc] init];
        [_view setFavorites:_favorites];
    }
    
    [_mainMenu removeAllItems];
    
    [_monitor setHideDisabledSensors:![_defaults boolForKey:@kHWMonitorShowHiddenSensors]];
    [_monitor setShowBSDNames:[_defaults boolForKey:@kHWMonitorShowBSDNames]];
    [_monitor setUseFahrenheit:[_defaults boolForKey:@kHWMonitorUseFahrenheitKey]];
    
    [_view setDrawValuesInRow:[_defaults boolForKey:@kHWMonitorFavoritesInRow]];
    [_view setUseShadowEffect:[_defaults boolForKey:@kHWMonitorUseShadowEffect]];
    
    [_monitor rebuildSensorsList];
    
    if ([[_monitor sensors] count] > 0) {
        
        NSUInteger i = 0;
        
        [_favorites removeAllObjects];
        
        NSMutableArray *favoritsList = [_defaults objectForKey:@kHWMonitorFavoritesList];
        
        if (favoritsList) {
            for (i = 0; i < [favoritsList count]; i++) {
                HWMonitorSensor *sensor = [[_monitor keys] objectForKey:[favoritsList objectAtIndex:i]];
                
                if (sensor) {
                    [sensor setFavorite:TRUE];
                    [_favorites addObject:sensor];
                }
            }
        }
        
        [self insertMenuGroupWithTitle:@"TEMPERATURES" Icon:_temperaturesIcon Sensors:[_monitor getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" Icon:_hddtemperaturesIcon Sensors:[_monitor getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" Icon:_ssdlifeIcon Sensors:[_monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING BLOCKS" Icon:_ssdlifeIcon Sensors:[_monitor getAllSensorsInGroup:kSMARTSensorGroupRemainingBlocks]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" Icon:_frequenciesIcon Sensors:[_monitor getAllSensorsInGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency]];
        [self insertMenuGroupWithTitle:@"FANS" Icon:_tachometersIcon Sensors:[_monitor getAllSensorsInGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" Icon:_voltagesIcon Sensors:[_monitor getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
        [_mainMenu addItem:[NSMenuItem separatorItem]];
        
        // Preferences...
        NSMenuItem* prefsItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(@"Preferences") action:nil keyEquivalent:@""];
        [prefsItem setImage:_prefsIcon];
        [_mainMenu addItem:prefsItem];
        
        _prefsMenu = [[NSMenu alloc] init];
        
        [self insertTitleItemWithMenu:_prefsMenu Title:@"GENERAL" Icon:nil];
        [self insertPrefsItemWithTitle:@"Show hidden sensors" icon:nil state:![_monitor hideDisabledSensors] action:@selector(showHiddenSensorsItemClicked:) keyEquivalent:@""];
        [self insertPrefsItemWithTitle:@"Use BSD drives names" icon:nil state:[_monitor showBSDNames] action:@selector(showBSDNamesItemClicked:) keyEquivalent:@""];
        
        [_prefsMenu addItem:[NSMenuItem separatorItem]];
        [self insertTitleItemWithMenu:_prefsMenu Title:@"MENU BAR" Icon:nil];
        [self insertPrefsItemWithTitle:@"Use big font" icon:nil state:[_view drawValuesInRow] action:@selector(favoritesInRowItemClicked:) keyEquivalent:@""];
        [self insertPrefsItemWithTitle:@"Use shadow effect" icon:nil state:[_view useShadowEffect] action:@selector(useShadowEffectItemClicked:) keyEquivalent:@""];
        
        
        [_prefsMenu addItem:[NSMenuItem separatorItem]];
        [self insertTitleItemWithMenu:_prefsMenu Title:@"TEMPERATURE SCALE" Icon:nil];
        _celsiusItem = [self insertPrefsItemWithTitle:@"Celsius" icon:nil state:![_monitor useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
        _fahrenheitItem = [self insertPrefsItemWithTitle:@"Fahrenheit" icon:nil state:[_monitor useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
        
        
        [prefsItem setSubmenu:_prefsMenu];
        
        [self updateTitlesForced];
    }
    else {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:NSLocalizedString(@"No sensors found", nil) action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [_mainMenu addItem:item];
    }
    
    if (!_isMenuExtra) {
        [_mainMenu addItem:[NSMenuItem separatorItem]];
        [_mainMenu addItem:[[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quit HWMonitor", nil) action:@selector(terminate:) keyEquivalent:@""]];
    }
}

- (void)sensorItemClicked:(id)sender 
{
    NSMenuItem * menuItem = (NSMenuItem *)sender;
    HWMonitorSensor *sensor = (HWMonitorSensor*)[menuItem representedObject];
    
    if ([sensor favorite])
        [_favorites removeObject:sensor];
    
    [sensor setFavorite:![sensor favorite]];
    [menuItem setState:[sensor favorite]];
    
    if ([sensor favorite])
        [_favorites addObject:sensor];
    
    [self updateTitlesDefault];
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    NSUInteger i;
    
    for (i = 0; i < [_favorites count]; i++) {
        sensor = [_favorites objectAtIndex:i];
        [list addObject:[sensor key]];
    }
    
    [_defaults setObject:list forKey:@kHWMonitorFavoritesList];
    [_defaults synchronize];
}

- (void)degreesItemClicked:(id)sender
{
    bool useFahrenheit = [sender isEqualTo:_fahrenheitItem];
    
    if (useFahrenheit != [_monitor useFahrenheit] ) {
        [_celsiusItem setState:!useFahrenheit];
        [_fahrenheitItem setState:useFahrenheit];
        
        [_monitor setUseFahrenheit:useFahrenheit];
        
        [_defaults setBool:useFahrenheit forKey:@kHWMonitorUseFahrenheitKey];
        [_defaults synchronize];
        
        [self updateTitlesForced];
    }
}

- (void)showHiddenSensorsItemClicked:(id)sender
{   
    [sender setState:![sender state]];
    
    [_defaults setBool:[sender state] forKey:@kHWMonitorShowHiddenSensors];
    [_defaults synchronize];
    
    [self rebuildSensors];
}

- (void)showBSDNamesItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [_defaults setBool:[sender state] forKey:@kHWMonitorShowBSDNames];
    [_defaults synchronize];
    
    [self rebuildSensors];
}

- (void)favoritesInRowItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [_view setDrawValuesInRow:[sender state]];
    
    [_defaults setBool:[sender state] forKey:@kHWMonitorFavoritesInRow];
    [_defaults synchronize];
}

- (void)useShadowEffectItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [_view setUseShadowEffect:[sender state]];
    
    [_defaults setBool:[sender state] forKey:@kHWMonitorUseShadowEffect];
    [_defaults synchronize];
}

- (void)systemWillSleep:(NSNotification *)aNotification
{
    //NSLog(@"receiveSleepNote: %@", [note name]);
    
    if ([aNotification name] == NSWorkspaceWillSleepNotification) {
        
    }
}

- (void)systemDidWake:(NSNotification *)aNotification
{
    //NSLog(@"receiveSleepNote: %@", [note name]);
    
    if ([aNotification name] == NSWorkspaceDidWakeNotification) {
        [self updateSMARTDataThreaded];
        [self updateDataThreaded];
    }
}

@end
