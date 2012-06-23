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

- (void)addAvailableItemsFromDictionary:(NSDictionary*)sensors inGroup:(NSUInteger)mainGroup
{
    HWMonitorIcon* icon = [self getIconByGroup:mainGroup];
    
    if (icon && ![_arrayController favoritesContainKey:[icon name]])
        [_arrayController addAvailableItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
    
    NSMutableArray *items = [[NSMutableArray alloc] init];
    
    for (NSDictionary *item in [sensors allValues]) {
        //NSString *name = [item valueForKey:@"Name"];
        //NSString *title = [item valueForKey:@"Title"];
        NSUInteger group = [[item valueForKey:@"Group"] intValue];
        BOOL favorite = [[item valueForKey:@"Favorite"] boolValue];
        
        if (mainGroup & group && !favorite)
            [items addObject:item];
    }
    
    [items sortUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        NSNumber *index1 = [obj1 valueForKey:@"Index"];
        NSNumber *index2 = [obj2 valueForKey:@"Index"];
        
        return [index1 compare:index2];
    }];
     
    for (NSDictionary *item in items) {
        NSString *name = [item valueForKey:@"Name"];
        NSString *title = [item valueForKey:@"Title"];
        NSUInteger group = [[item valueForKey:@"Group"] intValue];
        //BOOL favorite = [[item valueForKey:@"Favorite"] boolValue];
        icon = [self getIconByGroup:group];
        [_arrayController addAvailableItem:GetLocalizedString(title) icon:icon ? [icon image] : nil key:name];
    }    
}

- (void)recieveItems:(NSNotification*)aNotification
{
    [_arrayController removeAllItems];
    
    [_arrayController setFirstFavoriteItem:GetLocalizedString(@"Menu bar items:") firstAvailableItem:GetLocalizedString(@"Available items:")];
    
    if ([aNotification object] && [aNotification userInfo]) {
        NSArray *favoritesList = [(NSString*)[aNotification object] componentsSeparatedByString:@","];
        NSDictionary *sensorsList = [aNotification userInfo];
        
        for (NSString *favoriteName in favoritesList) {
            NSDictionary *item = [sensorsList valueForKey:favoriteName];
            
            if (item) {
                NSString *name = [item valueForKey:@"Name"];
                NSString *title = [item valueForKey:@"Title"];
                NSUInteger group = [[item valueForKey:@"Group"] intValue];
                BOOL favorite = [[item valueForKey:@"Favorite"] boolValue];
                HWMonitorIcon *icon = [self getIconByGroup:group];
                
                if (favorite)
                    [_arrayController addFavoriteItem:GetLocalizedString(title) icon:icon ? [icon image] : nil key:name];
            }
            else {
                HWMonitorIcon *icon = [self getIconByName:favoriteName];
                
                if (icon) {
                    [_arrayController addFavoriteItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
                }
            }
        }
        
        if (![_arrayController favoritesContainKey:kHWMonitorIconThermometer]) {
            
            HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconThermometer];
            
            if ([[_arrayController getFavoritesItems] count] == 0)
                [[_arrayController addFavoriteItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]] setValue:icon forKey:kHWMonitorRepresentedObject];
            else
                [[_arrayController addAvailableItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]] setValue:icon forKey:kHWMonitorRepresentedObject];
        }
        
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupTemperature];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupTemperature];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupRemainingLife];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupRemainingBlocks];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupVoltage];

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
        
        //[[NSWorkspace sharedWorkspace] openFile:[[NSBundle mainBundle] pathForResource:@"MenuCracker" ofType:@"menu"]];
        
        //sleep(1);
        
        //[[NSWorkspace sharedWorkspace] openFile:[[NSBundle mainBundle] pathForResource:@"HWMonitorExtra" ofType:@"menu"]];
 
        
        CoreMenuExtraAddMenuExtra((__bridge CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"MenuCracker" ofType:@"menu"]], 0, 0, 0, 0, 0);
        CoreMenuExtraAddMenuExtra((__bridge CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"HWMonitorExtra" ofType:@"menu"]], 0, 0, 0, 0, 0);
        
        menuExtra = nil;
        error = 0;
        int count = 0;
        
        do {
            count++;
            sleep(1);
            error = CoreMenuExtraGetMenuExtra(CFSTR("org.hwsensors.HWMonitorExtra"), &menuExtra);
        } while (!menuExtra && !error && count < 5);
        
        sleep(1);
        
        system("killall SystemUIServer");
    }
    
    [sender setEnabled:YES];
}

-(IBAction)favoritesChanged:(id)sender
{   
    NSArray *favorites = [_arrayController getFavoritesItems];
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    for (NSDictionary *item in favorites)
        [list addObject:[item valueForKey:@"Key"]];
    
    NSString* favoritesList = [list componentsJoinedByString:@","];
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorFavoritesChanged object:favoritesList userInfo:nil deliverImmediately:YES];
}

-(IBAction)useFahrenheitChanged:(id)sender
{
    NSMatrix *matrix = (NSMatrix*)sender;
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseFahrenheitChanged object:![[matrix cellAtRow:0 column:0] state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)useBigFontChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseBigFontChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)useShadowEffectChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseShadowsChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)showHiddenSensorsChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorShowHiddenChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)showBSDNamesChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorShowBSDNamesChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
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
    
    [[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(recieveItems:) name: HWMonitorRecieveItems object: NULL];
    
    // Request items
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorRequestItems object:nil userInfo:nil deliverImmediately:YES];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
}

@end
