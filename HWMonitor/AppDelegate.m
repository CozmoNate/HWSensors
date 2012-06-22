//
//  AppDelegate.m
//  HWMonitor
//
//  Created by Kozlek on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "SystemUIPlugin.h"
#import "HWMonitorDefinitions.h"
#import "HWMonitorIcon.h"

#import "AppDelegate.h"

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

#define kHWMonitorRepresentedObject @"RepresentedObject"

int CoreMenuExtraGetMenuExtra( CFStringRef identifier, void *menuExtra);
int CoreMenuExtraAddMenuExtra( CFURLRef path, int position, int whoCares, int whoCares2, int whoCares3, int whoCares4);
int CoreMenuExtraRemoveMenuExtra( void *menuExtra, int whoCares);

@implementation AppDelegate

@synthesize window = _window;
@synthesize engine = _engine;
@synthesize arrayController = _arrayController;
@synthesize toggleMenuButton = _toggleMenuButton;

- (void)loadIconNamed:(NSString*)name
{
    if (!_icons)
        _icons = [[NSMutableDictionary alloc] init];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:name ofType:@"png"]];
    NSImage *altImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[name stringByAppendingString:@"_template"] ofType:@"png"]];
    
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

- (void)addAvailableItemsFromGroup:(NSUInteger)group
{
    HWMonitorIcon* icon = [self getIconByGroup:group];
    
    if (icon && ![_arrayController favoritesContainKey:[icon name]])
        [[_arrayController addAvailableItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]] setValue:icon forKey:kHWMonitorRepresentedObject];
    
    for (HWMonitorSensor* sensor in [_engine sensors]) 
        if (group & [sensor group] && ![sensor favorite]) {
            icon = [self getIconByGroup:[sensor group]];
            [[_arrayController addAvailableItem:[sensor caption] icon:icon ? [icon image] : nil key:[sensor name]] setValue:sensor forKey:kHWMonitorRepresentedObject];
        }
}

- (void)rebuildSensors
{
    [_engine rebuildSensorsList];
    
    [_arrayController removeAllItems];
    
    if ([[_engine sensors] count] > 0) {
        [_arrayController setFirstFavoriteItem:GetLocalizedString(@"Menu bar items:") firstAvailableItem:GetLocalizedString(@"Available items:")];
        
        NSMutableArray *favoritesList = [_defaults objectForKey:kHWMonitorFavoritesList];
        
        if (favoritesList) {
            
            NSUInteger i = 0;
            
            for (i = 0; i < [favoritesList count]; i++) {
                
                NSString *name = [favoritesList objectAtIndex:i];
                
                HWMonitorSensor *sensor = nil;
                HWMonitorIcon *icon = nil;
                
                if ((sensor = [[_engine keys] objectForKey:name])) {
                    [sensor setFavorite:TRUE];
                    //[_favorites addObject:sensor];
                    icon = [self getIconByGroup:[sensor group]];
                    [[_arrayController addFavoriteItem:[sensor caption] icon:icon ? [icon image] : nil key:[sensor name]] setValue:sensor forKey:kHWMonitorRepresentedObject];
                }
                else if ((icon = [_icons objectForKey:name])) {
                    //[_favorites addObject:icon];
                    [[_arrayController addFavoriteItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]] setValue:icon forKey:kHWMonitorRepresentedObject];
                }
            }
        }
        
        if (![_arrayController favoritesContainKey:kHWMonitorIconThermometer]) {
            HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconThermometer];
            [[_arrayController addAvailableItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]] setValue:icon forKey:kHWMonitorRepresentedObject];
        }
        
        [self addAvailableItemsFromGroup:kHWSensorGroupTemperature];
        [self addAvailableItemsFromGroup:kSMARTSensorGroupTemperature];
        [self addAvailableItemsFromGroup:kSMARTSensorGroupRemainingLife];
        [self addAvailableItemsFromGroup:kSMARTSensorGroupRemainingBlocks];
        [self addAvailableItemsFromGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency];
        [self addAvailableItemsFromGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer];
        [self addAvailableItemsFromGroup:kHWSensorGroupVoltage];
    }
}

- (IBAction)toggleMenu:(id)sender
{
    [sender setEnabled:NO];
    
    void *menuExtra = nil;
    
    int error = CoreMenuExtraGetMenuExtra(CFSTR("org.hwsensors.HWMonitorExtra"), &menuExtra);
    
    if (error)
        return;
    
    if ((int)menuExtra > 0) {
        CoreMenuExtraRemoveMenuExtra(menuExtra, 0);
    }
    else {
        
        /*if (floor(NSAppKitVersionNumber) >= 900) {
            NSString *command = [NSString stringWithFormat:@"open -g %@" ,[[NSBundle mainBundle] pathForResource:@"HWMonitorExtra" ofType:@"menu"]];
            
			error = system([command cStringUsingEncoding:NSUTF16StringEncoding]); //nicier
		}
		else {
            
            NSString *command = [NSString stringWithFormat:@"open %@" ,[[NSBundle mainBundle] pathForResource:@"HWMonitorExtra" ofType:@"menu"]];
            
			error = system([command cStringUsingEncoding:NSUTF16StringEncoding]); //more compatible
        }*/
        
        [[NSWorkspace sharedWorkspace] openFile:[[NSBundle mainBundle] pathForResource:@"MenuCracker" ofType:@"menu"]];
        [[NSWorkspace sharedWorkspace] openFile:[[NSBundle mainBundle] pathForResource:@"HWMonitorExtra" ofType:@"menu"]];
 
        
        //CoreMenuExtraAddMenuExtra((__bridge CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"MenuCracker" ofType:@"menu"]], 0, 0, 0, 0, 0);
        //CoreMenuExtraAddMenuExtra((__bridge CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"HWMonitorExtra" ofType:@"menu"]], 0, 0, 0, 0, 0);
        
        menuExtra = nil;
        error = 0;
        int count = 0;
        
        while (!menuExtra && !error && count < 5) {
            error = CoreMenuExtraGetMenuExtra(CFSTR("org.hwsensors.HWMonitorExtra"), &menuExtra);
            sleep(1);
            count++;
        }
        
        sleep(1);
        
        system("killall SystemUIServer");
    }
    
    [sender setEnabled:YES];
}

-(IBAction)favoritesChanged:(id)sender
{
    for (HWMonitorSensor *sensor in [_engine sensors])
        [sensor setFavorite:NO];
    
    NSArray *favorites = [_arrayController getFavoritesItems];
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    NSUInteger i;
    
    for (i = 0; i < [favorites count]; i++) {

        id object = [[favorites objectAtIndex:i] valueForKey:kHWMonitorRepresentedObject];
        
        if ([object isKindOfClass:[HWMonitorIcon class]]) {
            
        }
        else if ([object isKindOfClass:[HWMonitorSensor class]]) {
            [object setFavorite:YES];
        }
        
        [list addObject:[object name]];
    }
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorFavoritesChanged object:[list componentsJoinedByString:@","]];
}

-(IBAction)useFahrenheitChanged:(id)sender
{
    NSMatrix *matrix = (NSMatrix*)sender;
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseFahrenheitChanged object:![[matrix cellAtRow:0 column:0] state] ? HWMonitorBooleanYES : HWMonitorBooleanNO];
}

-(IBAction)useBigFontChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseBigFontChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO];
}

-(IBAction)useShadowEffectChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseShadowsChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO];
}

-(IBAction)showHiddenSensorsChanged:(id)sender
{
    [_engine setHideDisabledSensors:![sender state]];
    
    [self rebuildSensors];
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorShowHiddenChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO];
}

-(IBAction)showBSDNamesChanged:(id)sender
{
    [_engine setShowBSDNames:[sender state]];
    
    [self rebuildSensors];
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorShowBSDNamesChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    void *menuExtra = nil;
    int state = CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
    
    [_toggleMenuButton setState:!state && (int)menuExtra > 0];
    
    _defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];
    
    // undocumented call
    [[NSUserDefaultsController sharedUserDefaultsController] _setDefaults:_defaults];
    
    [self loadIconNamed:kHWMonitorIconThermometer];
    [self loadIconNamed:kHWMonitorIconTemperatures];
    [self loadIconNamed:kHWMonitorIconHddTemperatures];
    [self loadIconNamed:kHWMonitorIconSsdLife];
    [self loadIconNamed:kHWMonitorIconMultipliers];
    [self loadIconNamed:kHWMonitorIconFrequencies];
    [self loadIconNamed:kHWMonitorIconTachometers];
    [self loadIconNamed:kHWMonitorIconVoltages];

    
    [_engine setHideDisabledSensors:![_defaults boolForKey:kHWMonitorShowHiddenSensors]];
    [_engine setShowBSDNames:[_defaults boolForKey:kHWMonitorShowBSDNames]];
    
    [self rebuildSensors];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

@end
