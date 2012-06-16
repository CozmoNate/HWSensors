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

@synthesize menu = _mainMenu;
@synthesize engine = _engine;
@synthesize view = _view;
@synthesize favorites = _favorites;

@synthesize prefsWindow = _prefsWindow;
@synthesize arrayController = _arrayController;

- (id)initWithStatusItem:(NSStatusItem*)item bundle:(NSBundle*)bundle;
{
    _statusItem = item;
    _bundle = bundle;
    
    if (_bundle == nil || _statusItem == nil) 
        return nil;
    
    //_isMenuExtra = [_statusItem isKindOfClass:[NSMenuExtra class]];
    _isMenuExtra = [_statusItem respondsToSelector:@selector(drawMenuBackground:)];
    
    _defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];
    
    // undocumented call
    [[NSUserDefaultsController sharedUserDefaultsController] _setDefaults:_defaults];
    
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
    
    [NSBundle loadNibNamed:@"PrefsMenu" owner:self];
    
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

- (HWMonitorIcon*)getIconByGroup:(NSUInteger)group
{
    if ((group & kHWSensorGroupTemperature) || (group & kSMARTSensorGroupTemperature)) {
        return [self getIconByName:kHWMonitorIconTemperatures];
    }
    else if ((group & kSMARTSensorGroupRemainingLife) || (group & kSMARTSensorGroupRemainingBlocks)) {
        return [self getIconByName:kHWMonitorIconSsdLife];
    }
    else if (group & kHWSensorGroupFrequency) {
        return [self getIconByName:kHWMonitorIconFrequencies];
    }
    else if (group & kHWSensorGroupMultiplier) {
        return [self getIconByName:kHWMonitorIconMultipliers];
    }
    else if ((group & kHWSensorGroupPWM) || (group & kHWSensorGroupTachometer)) {
        return [self getIconByName:kHWMonitorIconTachometers];
    }
    else if (group & kHWSensorGroupVoltage) {
        return [self getIconByName:kHWMonitorIconVoltages];
    }
    
    return nil;
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
        
        if (icon && ![_favorites containsObject:icon])
                [_arrayController addAvailableItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
        
        if ([[_mainMenu itemArray] count] > 0)
            [_mainMenu addItem:[NSMenuItem separatorItem]];
        
        NSMenuItem* titleItem = [self insertTitleItemWithMenu:_mainMenu Title:title Image:icon ? [icon image] : nil];
        
        //[titleItem setAction:@selector(titleItemClicked:)];
        //[titleItem setTarget:self];
        //[titleItem setRepresentedObject:icon];
        //[titleItem setOnStateImage:_favoriteIcon];
        //[titleItem setOffStateImage:_disabledIcon];
        //[titleItem setState:[_favorites containsObject:icon]];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[_favorites containsObject:sensor]];
            [sensor setTitle:[(NSString*)GetLocalizedString([sensor caption]) stringByTruncatingToWidth:145 withFont:_menuFont]];
            
            if (![sensor favorite])
                [_arrayController addAvailableItem:[sensor caption] icon:icon ? [icon image] : nil key:[sensor name]];
            
            NSMenuItem * sensorItem = [[NSMenuItem alloc] initWithTitle:[sensor title] action:nil/*@selector(sensorItemClicked:)*/ keyEquivalent:@""];
            
            [sensor setMenuItem:sensorItem];
            
            //[sensorItem setTarget:self];
            [sensorItem setRepresentedObject:sensor];
            //[sensorItem setState:[sensor favorite]];
            //[sensorItem setOnStateImage:_favoriteIcon];
            //[sensorItem setOffStateImage:_disabledIcon];
            
            [_mainMenu addItem:sensorItem];
        }
    }
}

/*- (NSMenuItem*)insertPrefsItemWithTitle:(NSString*)title icon:(NSImage*)image state:(NSUInteger)state action:(SEL)aSelector keyEquivalent:(NSString *)charCode
{
    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(title) action:aSelector keyEquivalent:charCode];
    
    [item setState:state];
    [item setTarget:self];
    
    [_prefsMenu addItem:item];
    
    return item;
}*/

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
    
    [_view setUseBigFont:[_defaults boolForKey:kHWMonitorUseBigStatusMenuFont]];
    [_view setUseShadowEffect:[_defaults boolForKey:kHWMonitorUseShadowEffect]];
    
    [_engine rebuildSensorsList];
    
    if ([[_engine sensors] count] > 0) {
        
        [_favorites removeAllObjects];
        [_arrayController setFirstFavoriteItem:GetLocalizedString(@"Menu bar items:") firstAvailableItem:GetLocalizedString(@"Available sensors:")];
        
        NSMutableArray *favoritsList = [_defaults objectForKey:kHWMonitorFavoritesList];
        
        if (favoritsList) {
            
            NSUInteger i = 0;
            
            for (i = 0; i < [favoritsList count]; i++) {
                
                NSString *name = [favoritsList objectAtIndex:i];
                
                HWMonitorSensor *sensor = nil;
                HWMonitorIcon *icon = nil;
                
                if ((sensor = [[_engine keys] objectForKey:name])) {
                    [sensor setFavorite:TRUE];
                    [_favorites addObject:sensor];
                    icon = [self getIconByGroup:[sensor group]];
                    [_arrayController addFavoriteItem:[sensor caption] icon:icon ? [icon image] : nil key:[sensor name]];
                }
                else if ((icon = [_icons objectForKey:name])) {
                    [_favorites addObject:icon];
                    [_arrayController addFavoriteItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
                }
            }
        }
        
        HWMonitorIcon *thermometer = [self getIconByName:kHWMonitorIconThermometer];
        
        if ([favoritsList count] == 0)
            [_arrayController addFavoriteItem:GetLocalizedString([thermometer name]) icon:[thermometer image] key:[thermometer name]];
        
        [self insertMenuGroupWithTitle:@"TEMPERATURES" Icon:[self getIconByName:kHWMonitorIconTemperatures] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"DRIVES TEMPERATURES" Icon:[self getIconByName:kHWMonitorIconHddTemperatures] Sensors:[_engine getAllSensorsInGroup:kSMARTSensorGroupTemperature]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING LIFE" Icon:[self getIconByName:kHWMonitorIconSsdLife] Sensors:[_engine getAllSensorsInGroup:kSMARTSensorGroupRemainingLife]];
        [self insertMenuGroupWithTitle:@"SSD REMAINING BLOCKS" Icon:[self getIconByName:kHWMonitorIconSsdLife] Sensors:[_engine getAllSensorsInGroup:kSMARTSensorGroupRemainingBlocks]];
        [self insertMenuGroupWithTitle:@"FREQUENCIES" Icon:[self getIconByName:kHWMonitorIconFrequencies] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency]];
        [self insertMenuGroupWithTitle:@"FANS" Icon:[self getIconByName:kHWMonitorIconTachometers] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer]];
        [self insertMenuGroupWithTitle:@"VOLTAGES" Icon:[self getIconByName:kHWMonitorIconVoltages] Sensors:[_engine getAllSensorsInGroup:kHWSensorGroupVoltage]];
        
        [_mainMenu addItem:[NSMenuItem separatorItem]];
        
        // Preferences...
        NSMenuItem* prefsItem = [[NSMenuItem alloc] initWithTitle:GetLocalizedString(@"Preferences...") action:@selector(prefsItemClicked:) keyEquivalent:@""];
        [prefsItem setImage:_prefsIcon];
        [prefsItem setTarget:self];
        [_mainMenu addItem:prefsItem];
        
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

- (void)prefsItemClicked:(id)sender
{
    if (_isMenuExtra)  
        [NSApp activateIgnoringOtherApps:YES];
    
    [_prefsWindow makeKeyAndOrderFront:sender];
}

- (IBAction)preferencesDidChanged:(id)sender
{
    if ([_defaults boolForKey:kHWMonitorUseFahrenheitKey] != [_engine useFahrenheit]) {
        [_engine setUseFahrenheit:[_defaults boolForKey:kHWMonitorUseFahrenheitKey]];
        [self updateTitlesForced];
    }
    
    if ([_defaults boolForKey:kHWMonitorUseBigStatusMenuFont] != [_view useBigFont]) {
        [_view setUseBigFont:[_defaults boolForKey:kHWMonitorUseBigStatusMenuFont]];
        [self updateTitlesForced];
    }
    
    if ([_defaults boolForKey:kHWMonitorUseShadowEffect] != [_view useShadowEffect]) {
        [_view setUseShadowEffect:[_defaults boolForKey:kHWMonitorUseShadowEffect]];
        [self updateTitlesForced];
    }
    
    [_favorites removeAllObjects];
    
    NSArray *favorites = [_arrayController getFavoritesItems];
    
    for (NSDictionary *item in favorites) {
        NSString *name = [item objectForKey:@"Key"];
        
        HWMonitorSensor *sensor = nil;
        HWMonitorIcon *icon = nil;
        
        if ((sensor = [[_engine keys] objectForKey:name])) {
            [sensor setFavorite:TRUE];
            [_favorites addObject:sensor];
            icon = [self getIconByGroup:[sensor group]];
        }
        else if ((icon = [_icons objectForKey:name])) {
            [_favorites addObject:icon];
        }
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

- (IBAction)forceRebuildSensors:(id)sender
{
    [self rebuildSensors];
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
