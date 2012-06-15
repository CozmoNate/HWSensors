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
@synthesize engine=_engine;
@synthesize favorites=_favorites;

- (id)initWithStatusItem:(NSStatusItem*)item bundle:(NSBundle*)bundle;
{
    _statusItem = item;
    _bundle = bundle;
    
    if (_bundle == nil || _statusItem == nil) 
        return nil;
    
    //_isMenuExtra = [_statusItem isKindOfClass:[NSMenuExtra class]];
    _isMenuExtra = [_statusItem respondsToSelector:@selector(drawMenuBackground:)];
    
    _defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];
    
    _favoriteIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"favorite" ofType:@"png"]];
    _disabledIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"disabled" ofType:@"png"]];
    _prefsIcon = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:@"preferences" ofType:@"png"]];
    
    [self loadIconNamed:kHWMonitorIconThermometer];
    
    [self loadIconNamed:kHWMonitorIconTemperatures];
    [self loadIconNamed:kHWMonitorIconHddTemperatures];
    [self loadIconNamed:kHWMonitorIconSsdLife];
    [self loadIconNamed:kHWMonitorIconMultipliers];
    [self loadIconNamed:kHWMonitorIconFrequencies];
    [self loadIconNamed:kHWMonitorIconTachometers];
    [self loadIconNamed:kHWMonitorIconVoltages];
    
    _view = [[HWMonitorView alloc] initWithFrame:NSMakeRect(0, 0, 24, 22) statusItem:_statusItem];
    
    HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconThermometer];
    
    [_view setImage:[icon image]];
    [_view setAlternateImage:[icon alternateImage]];
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

-(id)initWithMenuExtra:(NSMenuExtra *)item bundle:(NSBundle *)bundle
{
    _isMenuExtra = YES;
    
    return [self initWithStatusItem:item bundle:bundle];
}

- (void)loadIconNamed:(NSString*)name
{
    if (!_icons)
        _icons = [[NSMutableDictionary alloc] init];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:name ofType:@"png"]];
    NSImage *altImage = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForResource:[name stringByAppendingString:@"_template"] ofType:@"png"]];
    
    [_icons setObject:[HWMonitorIcon iconWithName:name image:image alternateImage:altImage] forKey:name];
}

- (HWMonitorIcon*)getIconByName:(NSString*)name
{
    return [_icons objectForKey:name];
}

- (NSMenuItem*)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Image:(NSImage *)image
{
    NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:GetLocalizedString(title)];
    
    [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
    [attributedTitle addAttribute:NSFontAttributeName value:_menuFont range:NSMakeRange(0, [attributedTitle length])];
 
    NSMenuItem *titleItem = [[NSMenuItem alloc] init];
    
    [titleItem setImage:image];
    [titleItem setAttributedTitle:attributedTitle];
    
    [someMenu addItem:titleItem];
    
    return titleItem;
}

- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(HWMonitorIcon*)icon Sensors:(NSArray *)list
{
    if (list && [list count] > 0) {
        if ([[_mainMenu itemArray] count] > 0)
            [_mainMenu addItem:[NSMenuItem separatorItem]];
        
        NSMenuItem* titleItem = [self insertTitleItemWithMenu:_mainMenu Title:title Image:icon ? [icon image] : nil];
        
        [titleItem setAction:@selector(titleItemClicked:)];
        [titleItem setTarget:self];
        [titleItem setRepresentedObject:icon];
        [titleItem setOnStateImage:_favoriteIcon];
        [titleItem setOffStateImage:_disabledIcon];
        [titleItem setState:[_favorites containsObject:icon]];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[_favorites containsObject:sensor]];
            [sensor setTitle:[(NSString*)GetLocalizedString([sensor caption]) stringByTruncatingToWidth:145 withFont:_menuFont]];
            
            NSMenuItem * sensorItem = [[NSMenuItem alloc] initWithTitle:[sensor title] action:@selector(sensorItemClicked:) keyEquivalent:@""];
            
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
    [_engine updateSMARTSensorsValues];
}

- (void)updateSMARTDataThreaded
{
    [self performSelectorInBackground:@selector(updateSMARTData) withObject:nil];
}

- (void)updateData
{
    [_engine updateGenericSensorsValuesButOnlyFavorits:![_view isMenuDown]];
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
    
    for (int i = 0; i < [[_engine sensors] count]; i++) {
        HWMonitorSensor *sensor = [[_engine sensors] objectAtIndex:i];
        
        if (sensor && ([_view isMenuDown] || allSensors) && [sensor valueHasBeenChanged]) {
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] init];
            
            NSDictionary *titleColor;
            NSDictionary *valueColor;
            
            NSString * value = [sensor value];
            
            switch ([sensor level]) {
                    /*case kHWSensorLevelDisabled:
                     break;
                     
                     case kHWSensorLevelNormal:
                     break;*/
                    
                case kHWSensorLevelModerate:
                    titleColor = _blackColorAttribute;
                    valueColor = _orangeColorAttribute;
                    break;
                    
                case kHWSensorLevelHigh:
                    titleColor = _blackColorAttribute;
                    valueColor = _redColorAttribute;
                    break;
                    
                case kHWSensorLevelExceeded:
                    titleColor = _redColorAttribute;
                    valueColor = _redColorAttribute;
                    break;
                    
                default:
                    titleColor = _blackColorAttribute;
                    valueColor = _blackColorAttribute;
                    break;
            }
            
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:[sensor title] attributes:titleColor]];
            [title appendAttributedString:[[NSAttributedString alloc] initWithString:@"\t" attributes:titleColor]];
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
    if (!_engine) {
        _engine = [[HWMonitorEngine alloc] initWithBundle:_bundle];
        [_view setEngine:_engine];
    }
    
    if (!_favorites) {
        _favorites = [[NSMutableArray alloc] init];
        [_view setFavorites:_favorites];
    }
    
    [_mainMenu removeAllItems];
    
    [_engine setHideDisabledSensors:![_defaults boolForKey:kHWMonitorShowHiddenSensors]];
    [_engine setShowBSDNames:[_defaults boolForKey:kHWMonitorShowBSDNames]];
    [_engine setUseFahrenheit:[_defaults boolForKey:kHWMonitorUseFahrenheitKey]];
    
    [_view setDrawValuesInRow:[_defaults boolForKey:kHWMonitorUseBigStatusMenuFont]];
    [_view setUseShadowEffect:[_defaults boolForKey:kHWMonitorUseShadowEffect]];
    
    [_engine rebuildSensorsList];
    
    if ([[_engine sensors] count] > 0) {
        
        // Save sensors data
        
        NSArray* icons = [[_icons allKeys]sortedArrayUsingComparator:^NSComparisonResult(NSString *a, NSString *b) {
            return [a compare:b];
        }];
        [_defaults setObject:icons forKey:kHWMonitorIconsList];
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        
        for (HWMonitorSensor* sensor in [_engine sensors]) {
            NSArray *value = [NSArray arrayWithObjects:[sensor caption], [NSNumber numberWithInt:[sensor group]], nil];
            
            [dictionary setObject:value forKey:[sensor name]];
        }
                          

        [_defaults setObject:dictionary forKey:kHWMonitorSensorsList];
        
        [_defaults synchronize];
        
        NSUInteger i = 0;
        
        [_favorites removeAllObjects];
        
        NSMutableArray *favoritsList = [_defaults objectForKey:kHWMonitorFavoritesList];
        
        if (favoritsList) {
            for (i = 0; i < [favoritsList count]; i++) {
                
                NSString *name = [favoritsList objectAtIndex:i];
                
                HWMonitorSensor *sensor = nil;
                HWMonitorIcon *icon = nil;
                
                if ((sensor = [[_engine keys] objectForKey:name])) {
                    [sensor setFavorite:TRUE];
                    [_favorites addObject:sensor];
                }
                else if ((icon = [_icons objectForKey:name])) {
                    [_favorites addObject:icon];
                }
            }
        }
        
        [self insertMenuGroupWithTitle:@"TEMPERATURES" Icon:[self getIconByName:kHWMonitorIconTemperatures] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" Icon:[self getIconByName:kHWMonitorIconHddTemperatures] Sensors:[_engine getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" Icon:[self getIconByName:kHWMonitorIconSsdLife] Sensors:[_engine getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING BLOCKS" Icon:[self getIconByName:kHWMonitorIconSsdLife] Sensors:[_engine getAllSensorsInGroup:kSMARTSensorGroupRemainingBlocks]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" Icon:[self getIconByName:kHWMonitorIconFrequencies] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency]];
        [self insertMenuGroupWithTitle:@"FANS" Icon:[self getIconByName:kHWMonitorIconTachometers] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" Icon:[self getIconByName:kHWMonitorIconVoltages] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
        [_mainMenu addItem:[NSMenuItem separatorItem]];
        
        // Preferences...
        NSMenuItem* prefsItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(@"Preferences") action:nil keyEquivalent:@""];
        [prefsItem setImage:_prefsIcon];
        [_mainMenu addItem:prefsItem];
        
        _prefsMenu = [[NSMenu alloc] init];
        
        [self insertTitleItemWithMenu:_prefsMenu Title:@"GENERAL" Image:nil];
        [self insertPrefsItemWithTitle:@"Show hidden sensors" icon:nil state:![_engine hideDisabledSensors] action:@selector(showHiddenSensorsItemClicked:) keyEquivalent:@""];
        [self insertPrefsItemWithTitle:@"Use BSD drives names" icon:nil state:[_engine showBSDNames] action:@selector(showBSDNamesItemClicked:) keyEquivalent:@""];
        
        [_prefsMenu addItem:[NSMenuItem separatorItem]];
        [self insertTitleItemWithMenu:_prefsMenu Title:@"MENU BAR" Image:nil];
        [self insertPrefsItemWithTitle:@"Use big font" icon:nil state:[_view drawValuesInRow] action:@selector(favoritesInRowItemClicked:) keyEquivalent:@""];
        [self insertPrefsItemWithTitle:@"Use shadow effect" icon:nil state:[_view useShadowEffect] action:@selector(useShadowEffectItemClicked:) keyEquivalent:@""];
        
        
        [_prefsMenu addItem:[NSMenuItem separatorItem]];
        [self insertTitleItemWithMenu:_prefsMenu Title:@"TEMPERATURE SCALE" Image:nil];
        _celsiusItem = [self insertPrefsItemWithTitle:@"Celsius" icon:nil state:![_engine useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
        _fahrenheitItem = [self insertPrefsItemWithTitle:@"Fahrenheit" icon:nil state:[_engine useFahrenheit] action:@selector(degreesItemClicked:) keyEquivalent:@""];
        
        
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

- (void)titleItemClicked:(id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *)sender;
    HWMonitorIcon *icon = (HWMonitorIcon*)[menuItem representedObject];
    
    if ([_favorites containsObject:icon]) {
        [_favorites removeObject:icon];
        [menuItem setState:NO];
    }
    else {
        [_favorites addObject:icon];
        [menuItem setState:YES];
    }
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    NSUInteger i;
    
    for (i = 0; i < [_favorites count]; i++) {
        id object = [_favorites objectAtIndex:i];
        [list addObject:[object name]];
    }
    
    [_defaults setObject:list forKey:kHWMonitorFavoritesList];
    [_defaults synchronize];
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
        id object = [_favorites objectAtIndex:i];
        [list addObject:[object name]];
    }
    
    [_defaults setObject:list forKey:kHWMonitorFavoritesList];
    [_defaults synchronize];
}

- (void)degreesItemClicked:(id)sender
{
    bool useFahrenheit = [sender isEqualTo:_fahrenheitItem];
    
    if (useFahrenheit != [_engine useFahrenheit] ) {
        [_celsiusItem setState:!useFahrenheit];
        [_fahrenheitItem setState:useFahrenheit];
        
        [_engine setUseFahrenheit:useFahrenheit];
        
        [_defaults setBool:useFahrenheit forKey:kHWMonitorUseFahrenheitKey];
        [_defaults synchronize];
        
        [self updateTitlesForced];
    }
}

- (void)showHiddenSensorsItemClicked:(id)sender
{   
    [sender setState:![sender state]];
    
    [_defaults setBool:[sender state] forKey:kHWMonitorShowHiddenSensors];
    [_defaults synchronize];
    
    [self rebuildSensors];
}

- (void)showBSDNamesItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [_defaults setBool:[sender state] forKey:kHWMonitorShowBSDNames];
    [_defaults synchronize];
    
    [self rebuildSensors];
}

- (void)favoritesInRowItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [_view setDrawValuesInRow:[sender state]];
    
    [_defaults setBool:[sender state] forKey:kHWMonitorUseBigStatusMenuFont];
    [_defaults synchronize];
}

- (void)useShadowEffectItemClicked:(id)sender
{
    [sender setState:![sender state]];
    
    [_view setUseShadowEffect:[sender state]];
    
    [_defaults setBool:[sender state] forKey:kHWMonitorUseShadowEffect];
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
