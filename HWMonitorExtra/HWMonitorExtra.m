//
//  HWMonitorExtra.m
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//


#import "NSString+TruncateToWidth.h"
#import "HWMonitorDefinitions.h"

#import "HWMonitorExtra.h"

#define GetLocalizedString(key) \
[[self bundle] localizedStringForKey:(key) value:@"" table:nil]

@implementation HWMonitorExtra

@synthesize engine = _engine;
@synthesize favorites = _favorites;

- (id)initWithBundle:(NSBundle *)bundle
{
    self = [super initWithBundle:bundle];
    
    if (self == nil) return nil;
    
    _defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];
    
    // undocumented call
    [[NSUserDefaultsController sharedUserDefaultsController] _setDefaults:_defaults];
    
    _favoriteIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"favorite" ofType:@"png"]];
    _disabledIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"disabled" ofType:@"png"]];
    _prefsIcon = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"preferences" ofType:@"png"]];
    
    [self loadIconNamed:kHWMonitorIconThermometer];
    [self loadIconNamed:kHWMonitorIconTemperatures];
    [self loadIconNamed:kHWMonitorIconHddTemperatures];
    [self loadIconNamed:kHWMonitorIconSsdLife];
    [self loadIconNamed:kHWMonitorIconMultipliers];
    [self loadIconNamed:kHWMonitorIconFrequencies];
    [self loadIconNamed:kHWMonitorIconTachometers];
    [self loadIconNamed:kHWMonitorIconVoltages];
    
    HWMonitorView *view = [[HWMonitorView alloc] initWithFrame:[[self view] frame] menuExtra:self];
    
    HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconThermometer];
    
    [view setImage:[icon image]];
    [view setAlternateImage:[icon alternateImage]];
    [view setUseShadowEffect:YES];
    
    [self setView:view];
    
    _mainMenu = [[NSMenu alloc] init];
    
    [_mainMenu setAutoenablesItems: NO];
    [_mainMenu setDelegate:(id<NSMenuDelegate>)[self view]];
    
    _menuFont = [NSFont boldSystemFontOfSize:10.0];
    [_mainMenu setFont:_menuFont];
    
    [self setMenu:_mainMenu];
    
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
    /*invocation = [NSInvocation invocationWithMethodSignature:
                  [self methodSignatureForSelector:@selector(rebuildSensors)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(rebuildSensors)];*/
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:1800.0 invocation:invocation repeats:YES] forMode:NSDefaultRunLoopMode];
    
    // Register PM events
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(systemWillSleep:) name: NSWorkspaceWillSleepNotification object: NULL];
    
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(systemDidWake:) name: NSWorkspaceDidWakeNotification object: NULL];
    
    // Register HWMonitorNotifications
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(favoritesChanged:) name: HWMonitorFavoritesChanged object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(useFahrenheitChanged:) name: HWMonitorUseFahrenheitChanged object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(useBigFontChanged:) name: HWMonitorUseBigFontChanged object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(useShadowEffectChanged:) name: HWMonitorUseShadowsChanged object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(showHiddenSensorsChanged:) name: HWMonitorShowHiddenChanged object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(showBSDNamesChanged:) name: HWMonitorShowBSDNamesChanged object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(itemsRequested:) name: HWMonitorRequestItems object: NULL];
    
    [self performSelector:@selector(rebuildSensors) withObject:nil afterDelay:0.0];
    
    [self setView:view];
    
    return self;
}

- (void)loadIconNamed:(NSString*)name
{
    if (!_icons)
        _icons = [[NSMutableDictionary alloc] init];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:name ofType:@"png"]];
    NSImage *altImage = [[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:[name stringByAppendingString:@"_template"] ofType:@"png"]];
    
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

- (void)updateSensorTitle:(HWMonitorSensor*)sensor
{
    [sensor setTitle:[(NSString*)GetLocalizedString([sensor caption]) stringByTruncatingToWidth:145 withFont:_menuFont]];
}

- (NSMenuItem*)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Image:(NSImage *)image
{
    NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:GetLocalizedString(title)];
    
    [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
    [attributedTitle addAttribute:NSFontAttributeName value:_menuFont range:NSMakeRange(0, [attributedTitle length])];
    
    NSMenuItem *titleItem = [[NSMenuItem alloc] init];
    
    [titleItem setImage:image];
    [titleItem setAttributedTitle:attributedTitle];
    [titleItem setEnabled:NO];
    
    [someMenu addItem:titleItem];
    
    return titleItem;
}

- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(HWMonitorIcon*)icon Sensors:(NSArray *)list
{    
    if (list && [list count] > 0) {
                
        if ([[_mainMenu itemArray] count] > 0)
            [_mainMenu addItem:[NSMenuItem separatorItem]];
        
        [self insertTitleItemWithMenu:_mainMenu Title:title Image:icon ? [icon image] : nil];
        
        for (int i = 0; i < [list count]; i++) {
            HWMonitorSensor *sensor = (HWMonitorSensor*)[list objectAtIndex:i];
            
            [sensor setFavorite:[_favorites containsObject:sensor]];
            
            [self updateSensorTitle:sensor];
                        
            NSMenuItem * sensorItem = [[NSMenuItem alloc] initWithTitle:[sensor title] action:nil/*@selector(sensorItemClicked:)*/ keyEquivalent:@""];
            [sensorItem setEnabled:NO];
            [sensorItem setRepresentedObject:sensor];
            
            [_mainMenu addItem:sensorItem];
            [sensor setMenuItem:sensorItem];
        }
    }
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
    [_engine updateGenericSensorsValuesButOnlyFavorits:![self isMenuDown]];
}

- (void)updateDataThreaded
{
    [self performSelectorInBackground:@selector(updateData) withObject:nil];
}

- (void)updateTitlesForceAllSensors:(BOOL)allSensors
{    
    [[self view] setNeedsDisplay:YES];
    
    if (!allSensors && ![self isMenuDown]) 
        return;
    
    for (int i = 0; i < [[_engine sensors] count]; i++) {
        HWMonitorSensor *sensor = [[_engine sensors] objectAtIndex:i];
        
        if (sensor && ([self isMenuDown] || allSensors) && [sensor valueHasBeenChanged]) {
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

- (void)openPreferences:(id)sender
{
    [[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:@"org.hwsensors.HWMonitor" options:NSWorkspaceLaunchAsync additionalEventParamDescriptor:nil launchIdentifier:nil];
    //[[NSWorkspace sharedWorkspace] launchApplication:@"/Applications/HWMonitor.app"];
}

- (void)rebuildSensors
{
    if (!_engine) {
        _engine = [[HWMonitorEngine alloc] initWithBundle:[self bundle]];
        [(HWMonitorView*)[self view] setEngine:_engine];
    }
    
    if (!_favorites) {
        _favorites = [[NSMutableArray alloc] init];
        [(HWMonitorView*)[self view] setFavorites:_favorites];
    }
    
    [_mainMenu removeAllItems];
    
    [_engine setHideDisabledSensors:![_defaults boolForKey:kHWMonitorShowHiddenSensors]];
    [_engine setShowBSDNames:[_defaults boolForKey:kHWMonitorShowBSDNames]];
    [_engine setUseFahrenheit:[_defaults boolForKey:kHWMonitorUseFahrenheitKey]];
    
    [(HWMonitorView*)[self view] setUseBigFont:[_defaults boolForKey:kHWMonitorUseBigStatusMenuFont]];
    [(HWMonitorView*)[self view] setUseShadowEffect:[_defaults boolForKey:kHWMonitorUseShadowEffect]];
    
    [_engine rebuildSensorsList];
    
    if ([[_engine sensors] count] > 0) {
        
        [_favorites removeAllObjects];

        NSMutableArray *favoritesList = [_defaults objectForKey:kHWMonitorFavoritesList];
        
        if (favoritesList) {
            
            NSUInteger i = 0;
            
            for (i = 0; i < [favoritesList count]; i++) {
                
                NSString *name = [favoritesList objectAtIndex:i];
                
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
        
        NSMenuItem * prefsItem = [[NSMenuItem alloc]initWithTitle:GetLocalizedString(@"Preferences...") action:@selector(openPreferences:) keyEquivalent:@""];
        
        [prefsItem setImage:[[NSImage alloc] initWithContentsOfFile:[[self bundle] pathForResource:@"preferences" ofType:@"png"]]];
        [prefsItem setTarget:self];
        
        [_mainMenu addItem:prefsItem];
        
        [self updateTitlesForced];
    }
    else {
        NSMenuItem * item = [[NSMenuItem alloc]initWithTitle:GetLocalizedString(@"No sensors found") action:nil keyEquivalent:@""];
        
        [item setEnabled:FALSE];
        
        [_mainMenu addItem:item];
    }
    
    [self itemsRequested:nil];
}

- (void)itemsRequested:(NSNotification*)aNotification
{
    //NSLog(@"Sending items by request...");
    
    NSMutableArray *favoritesList = [[NSMutableArray alloc] init];
    
    for (id object in _favorites)
        [favoritesList addObject:[object name]];
    
    NSMutableDictionary *sensorsList = [[NSMutableDictionary alloc] init];
    
    NSUInteger index = 0;
    
    for (HWMonitorSensor *sensor in [_engine sensors]) {
        [sensorsList setValue:[NSDictionary dictionaryWithObjectsAndKeys:
                               [sensor name], @"Name",
                               [sensor caption], @"Title",
                               [NSNumber numberWithInt:[sensor group]], @"Group",
                               [NSNumber numberWithBool:[sensor favorite]], @"Favorite",
                               [NSNumber numberWithInt:index], @"Index",
                               nil] forKey:[sensor name]];
        
        index++;
    }
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorRecieveItems object:[favoritesList componentsJoinedByString:@","] userInfo:sensorsList deliverImmediately:YES];
}

- (void)favoritesChanged:(NSNotification*)aNotification
{
    for (id object in _favorites)
        if ([object isKindOfClass:[HWMonitorSensor class]]) 
            [object setFavorite:NO];
    
    [_favorites removeAllObjects];
    
    NSArray *favoritesList = [(NSString*)[aNotification object] componentsSeparatedByString:@","];
    
    if (favoritesList) {
        
        NSUInteger i = 0;
        
        for (i = 0; i < [favoritesList count]; i++) {
            
            NSString *name = [favoritesList objectAtIndex:i];
            
            HWMonitorSensor *sensor = [[_engine keys] objectForKey:name];
            HWMonitorIcon *icon = [_icons objectForKey:name];
            
            if (icon)
                [_favorites addObject:icon];
            
            if (sensor) {
                [sensor setFavorite:TRUE];
                [_favorites addObject:sensor];
            }
        }
    }
    
    [[self view] setNeedsDisplay:YES];
    
    [_defaults setObject:favoritesList forKey:kHWMonitorFavoritesList];
    [_defaults synchronize];
}

- (void)useFahrenheitChanged:(NSNotification*)aNotification
{
    BOOL useFahrenheit = [aNotification object] && [[aNotification object] isKindOfClass:[NSString class]] ? [(NSString*)[aNotification object] isEqualToString:HWMonitorBooleanYES] : NO;
    
    [_engine setUseFahrenheit:useFahrenheit];
    [self updateTitlesForced];
    [[self view] setNeedsDisplay:YES];
    
    [_defaults setBool:useFahrenheit forKey:kHWMonitorUseFahrenheitKey];
    [_defaults synchronize];
}

- (void)useBigFontChanged:(NSNotification*)aNotification
{   
    BOOL useBigFont = [aNotification object] && [[aNotification object] isKindOfClass:[NSString class]] ? [(NSString*)[aNotification object] isEqualToString:HWMonitorBooleanYES] : NO;
    
    [(HWMonitorView*)[self view] setUseBigFont:useBigFont];
    [self updateTitlesForced];
    [[self view] setNeedsDisplay:YES];
    
    [_defaults setBool:useBigFont forKey:kHWMonitorUseBigStatusMenuFont];
    [_defaults synchronize];
}

- (void)useShadowEffectChanged:(NSNotification*)aNotification
{
    BOOL useShadowEffect = [aNotification object] && [[aNotification object] isKindOfClass:[NSString class]] ? [(NSString*)[aNotification object] isEqualToString:HWMonitorBooleanYES] : NO;
    
    [(HWMonitorView*)[self view] setUseShadowEffect:useShadowEffect];
    [self updateTitlesForced];
    [[self view] setNeedsDisplay:YES];
    
    [_defaults setBool:useShadowEffect forKey:kHWMonitorUseShadowEffect];
    [_defaults synchronize];
}

- (void)showHiddenSensorsChanged:(NSNotification*)aNotification
{
    BOOL showHiddenSensors = [aNotification object] && [[aNotification object] isKindOfClass:[NSString class]] ? [(NSString*)[aNotification object] isEqualToString:HWMonitorBooleanYES] : NO;
    
    [_defaults setBool:showHiddenSensors forKey:kHWMonitorShowHiddenSensors];
    [_defaults synchronize];
    
    [_engine setHideDisabledSensors:!showHiddenSensors];
    
    [self rebuildSensors];
}

- (void)showBSDNamesChanged:(NSNotification*)aNotification
{
    BOOL showBSDNames = [aNotification object] && [[aNotification object] isKindOfClass:[NSString class]] ? [(NSString*)[aNotification object] isEqualToString:HWMonitorBooleanYES] : NO;
    
    [_defaults setBool:showBSDNames forKey:kHWMonitorShowBSDNames];
    [_defaults synchronize];
    
    [_engine setShowBSDNames:showBSDNames];
    
    for (HWMonitorSensor *sensor in [_engine sensors]) 
        [self updateSensorTitle:sensor];
    
    [self itemsRequested:nil];    
}

- (void)systemWillSleep:(NSNotification *)aNotification
{
    //
}

- (void)systemDidWake:(NSNotification *)aNotification
{
    [self updateSMARTDataThreaded];
    [self updateDataThreaded];
}

- (void)willUnload
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
}

@end
