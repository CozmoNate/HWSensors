//
//  AppDelegate.m
//  HWMonitor
//
//  Created by kozlek on 22.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "SystemUIPlugin.h"
#import "HWMonitorDefinitions.h"
#import "HWMonitorIcon.h"

#import "AppDelegate.h"

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

int CoreMenuExtraGetMenuExtra( CFStringRef identifier, void *menuExtra);
int CoreMenuExtraAddMenuExtra( CFURLRef path, int position, int whoCares, int whoCares2, int whoCares3, int whoCares4);
int CoreMenuExtraRemoveMenuExtra( void *menuExtra, int whoCares);

@implementation AppDelegate

@synthesize window = _window;
@synthesize menu = _menu;
@synthesize arrayController = _arrayController;
@synthesize versionLabel = _versionLabel;
@synthesize toggleMenuButton = _toggleMenuButton;

@synthesize userInterfaceEnabled = _userInterfaceEnabled;

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

        icon = [self getIconByGroup:[[item valueForKey:@"Group"] intValue]];

        NSMutableDictionary *availableItem = [_arrayController addAvailableItem];
        
        [availableItem setValuesForKeysWithDictionary:
         [NSDictionary dictionaryWithObjectsAndKeys:
          [item valueForKey:@"Title"], kHWMonitorKeyName,
          icon ? [icon image] : nil, kHWMonitorKeyIcon,
          [item valueForKey:@"Value"], kHWMonitorKeyValue,
          [item valueForKey:@"Visible"], kHWMonitorKeyVisible,
          [item valueForKey:@"Name"], kHWMonitorKeyKey,
          nil]];
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
            
            HWMonitorIcon *icon = [self getIconByName:favoriteName];
            
            if (icon) {
                [_arrayController addFavoriteItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
            }
            else {
                NSDictionary *item = [sensorsList valueForKey:favoriteName];
                
                if (item) {

                    icon = [self getIconByGroup:[[item valueForKey:@"Group"] intValue]];
                    
                    NSMutableDictionary *favoriteItem = [_arrayController addFavoriteItem];
                    
                    [favoriteItem setValuesForKeysWithDictionary:
                     [NSDictionary dictionaryWithObjectsAndKeys:
                      [item valueForKey:@"Title"], kHWMonitorKeyName,
                      icon ? [icon image] : nil, kHWMonitorKeyIcon,
                      [item valueForKey:@"Value"], kHWMonitorKeyValue,
                      [item valueForKey:@"Visible"], kHWMonitorKeyVisible,
                      [item valueForKey:@"Name"], kHWMonitorKeyKey,
                      nil]];
                }
            }
        }
        
        if (![_arrayController favoritesContainKey:kHWMonitorIconThermometer]) {
            
            HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconThermometer];
            
            if ([[_arrayController getFavoritesItems] count] == 0)
                [_arrayController addFavoriteItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
            else
                [_arrayController addAvailableItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
        }
        
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupTemperature];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupTemperature];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupRemainingLife];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupRemainingBlocks];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer];
        [self addAvailableItemsFromDictionary:sensorsList inGroup:kHWSensorGroupVoltage];

        [self setUserInterfaceEnabled:[[NSObject alloc] init]];
    }
    else {
        [self setUserInterfaceEnabled:nil];
    }
}

-(void)valuesChanged:(NSNotification *)aNotification
{
    if ([aNotification userInfo]) {
        NSDictionary *list = [aNotification userInfo];
        
        for (NSMutableDictionary *item in [_arrayController arrangedObjects]) {
            
            NSString *key = [item objectForKey:kHWMonitorKeyKey];
            
            if ([[list allKeys] containsObject:key]) {
                NSString *value = [list objectForKey:key];
                
                [item setValue:value forKey:kHWMonitorKeyValue];
            }
        }
    }
}

- (IBAction)toggleMenu:(id)sender
{
    void *menuExtra = nil;
    
    int error = CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
    
    if (error) {
        [sender setState:NO];
        return;
    }
    
    if ((int)menuExtra > 0) {
        CoreMenuExtraRemoveMenuExtra(menuExtra, 0);
        [self setUserInterfaceEnabled:nil];
        [sender setState:NO];
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
 
        [sender setEnabled:NO];
        
        CoreMenuExtraAddMenuExtra((__bridge CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"MenuCracker" ofType:@"menu"]], 0, 0, 0, 0, 0);
        CoreMenuExtraAddMenuExtra((__bridge CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"HWMonitorExtra" ofType:@"menu"]], 0, 0, 0, 0, 0);
        
        menuExtra = nil;
        error = 0;
        int count = 0;
        
        do {
            count++;
            sleep(1);
            error = CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
        } while ((!menuExtra || error) && count < 3);
        
        [sender setState:!error && menuExtra];
        [sender setEnabled:YES];
    }
}

-(IBAction)favoritesChanged:(id)sender
{   
    NSArray *favorites = [_arrayController getFavoritesItems];
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    for (NSDictionary *item in favorites)
        [list addObject:[item valueForKey:@"Key"]];
    
    NSString *favoritesList = [list componentsJoinedByString:@","];
    
    NSArray *items = [_arrayController getAllItems];
    NSMutableDictionary *info = [[NSMutableDictionary alloc] init];
    
    for (NSDictionary *item in items) {
        NSNumber *visible = [item valueForKey:@"Visible"];
        
        if (visible) {
            [info setObject:visible forKey:[item valueForKey:@"Key"]];
        }
    }
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorFavoritesChanged object:favoritesList userInfo:info deliverImmediately:YES];
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

-(IBAction)showBSDNamesChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorShowBSDNamesChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(void)localizeView:(NSView*)view
{
    if ([view isKindOfClass:[NSMatrix class]]) {
        NSMatrix *matrix = (NSMatrix*)view;
        
        NSUInteger row, column;
        
        for (row = 0 ; row < [matrix numberOfRows]; row++) {
            for (column = 0; column < [matrix numberOfColumns] ; column++) {
                NSButtonCell* cell = [matrix cellAtRow:row column:column];
                
                NSString *title = [cell title];
                
                [cell setTitle:GetLocalizedString(title)];
            }
        }
    }
    else if ([view isKindOfClass:[NSButton class]]) {
        NSButton *button = (NSButton*)view;
        
        NSString *title = [button title];

        [button setTitle:GetLocalizedString(title)];
        [button setAlternateTitle:GetLocalizedString([button alternateTitle])];
    }
    else if ([view isKindOfClass:[NSTextField class]]) {
        NSTextField *textField = (NSTextField*)view;
        
        NSString *title = [[textField cell] title];
        
        [[textField cell] setTitle:GetLocalizedString(title)];
    }
    else {
        if ([view respondsToSelector:@selector(setAlternateTitle:)]) {
            NSString *title = [(id)view alternateTitle];
            [(id)view setAlternateTitle:GetLocalizedString(title)];
        }
        if ([view respondsToSelector:@selector(setTitle:)]) {
            NSString *title = [(id)view title];
            [(id)view setTitle:GetLocalizedString(title)];
        }
    }
    
    for(NSView *subView in [view subviews])
        [self localizeView:subView];
}

-(void)localizeMenu:(NSMenu*)menu
{
    if (menu) {
        [menu setTitle:GetLocalizedString([menu title])];

        for (id subItem in [menu itemArray]) {
            if ([subItem isKindOfClass:[NSMenuItem class]]) {
                NSMenuItem* menuItem = subItem;
                
                [menuItem setTitle:GetLocalizedString([menuItem title])];
                
                if ([menuItem hasSubmenu])
                    [self localizeMenu:[menuItem submenu]];
            }
        }
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    void *menuExtra = nil;
    int error = CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
    
    [_toggleMenuButton setState:!error && menuExtra];
    
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
    
    // Update version label
	NSString *version = [[NSString alloc] initWithFormat:@"v%@ (%@)", [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"], [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"]];
    
    [_versionLabel setTitleWithMnemonic:version];
    
    [_window setTitle:GetLocalizedString([_window title])];
    
    [self localizeView:[_window contentView]];
    [self localizeMenu:_menu];
    
    [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(recieveItems:) name:HWMonitorRecieveItems object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(valuesChanged:) name:HWMonitorValuesChanged object: NULL];
    
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
