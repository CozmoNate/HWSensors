//
//  AppDelegate.m
//  HWMonitor
//
//  Created by kozlek on 22.06.12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.


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
@synthesize preferencesView = _preferencesView;
@synthesize favoritesView = _favoritesView;
@synthesize graphsView = _graphsView;
@synthesize prefsToolbar = _prefsToolbar;
@synthesize menu = _menu;
@synthesize sensorsController = _sensorsController;
@synthesize graphsController = _graphsController;
@synthesize versionLabel = _versionLabel;
@synthesize toggleMenuButton = _toggleMenuButton;

@synthesize sensorsUpdateRateText = _sensorsUpdateRateText;
@synthesize sensorsUpdateRateSlider = _sensorsUpdateRateSlider;
@synthesize SMARTUpdateRateText = _SMARTUpdateRateText;
@synthesize SMARTUpdateRateSlider = _SMARTUpdateRateSlider;

@synthesize temperatureGraph = _temperatureGraph;
@synthesize frequencyGraph = _frequencyGraph;
@synthesize tachometerGraph = _tachometerGraph;
@synthesize voltageGraph = _voltageGraph;

@synthesize userInterfaceEnabled = _userInterfaceEnabled;

- (void)loadIconNamed:(NSString*)name AsTemplate:(BOOL)template
{
    if (!_icons)
        _icons = [[NSMutableDictionary alloc] init];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:name ofType:@"png"]];
    
    [image setTemplate:template];
    
    NSImage *altImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[name stringByAppendingString:@"_template"] ofType:@"png"]];
    
    [altImage setTemplate:YES];
    
    [_icons setObject:[HWMonitorIcon iconWithName:name image:image alternateImage:altImage] forKey:name];
}

- (void)addIconNamed:(NSString*)name FromImage:(NSImage*)image
{
    if (!image)
        return;
    
    if (!_icons)
        _icons = [[NSMutableDictionary alloc] init];
    
    NSImage *altImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[name stringByAppendingString:@"@blue"] ofType:@"png"]];
    
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

- (void)addItemsFromDictionary:(NSDictionary*)sensors inGroup:(NSUInteger)mainGroup
{
    HWMonitorIcon* icon = [self getIconByGroup:mainGroup];
    
    if (icon && ![_sensorsController favoritesContainKey:[icon name]])
        [_sensorsController addItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
    
    NSMutableArray *items = [[NSMutableArray alloc] init];
    
    for (NSDictionary *item in [sensors allValues]) {
        NSUInteger group = [[item valueForKey:kHWMonitorKeyGroup] intValue];
        
        if (mainGroup & group)
            [items addObject:item];
    }
    
    [items sortUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        NSNumber *index1 = [obj1 valueForKey:kHWMonitorKeyIndex];
        NSNumber *index2 = [obj2 valueForKey:kHWMonitorKeyIndex];
        
        return [index1 compare:index2];
    }];
        
    for (NSDictionary *item in items) {

        NSString *title = [item valueForKey:kHWMonitorKeyTitle];
        NSString *value = [item valueForKey:kHWMonitorKeyValue];
        NSString *key = [item valueForKey:kHWMonitorKeyName];
        //NSNumber *favorite = [item valueForKey:kHWMonitorKeyFavorite];
        
        icon = [self getIconByGroup:[[item valueForKey:kHWMonitorKeyGroup] intValue]];

        //if (![favorite boolValue]) {
            //NSMutableDictionary *availableItem = [_prefsController addAvailableItem];
            NSMutableDictionary *availableItem = [_sensorsController addItem];
            
            [availableItem setValuesForKeysWithDictionary:
             [NSDictionary dictionaryWithObjectsAndKeys:
              title,                                        kHWMonitorKeyName,
              icon ? [icon image] : nil,                    kHWMonitorKeyIcon,
              value,                                        kHWMonitorKeyValue,
              [item valueForKey:kHWMonitorKeyVisible],      kHWMonitorKeyVisible,
              key,                                          kHWMonitorKeyKey,
                                                            nil]];
        //}
        
        NSUInteger group = [[item valueForKey:kHWMonitorKeyGroup] intValue];
        
        if (group & kHWSensorGroupTemperature ||
            group & kSMARTSensorGroupTemperature ||
            group & kHWSensorGroupFrequency ||
            group & kHWSensorGroupTachometer ||
            group & kHWSensorGroupVoltage) {
            
            [_graphsController addObject:[NSMutableDictionary dictionaryWithObjectsAndKeys:
                                          [_globalColors objectAtIndex:_globalColorIndex], kHWMonitorKeyColor,
                                          icon ? [icon image] : nil, kHWMonitorKeyIcon,
                                          [NSNumber numberWithBool:YES], kHWMonitorKeyEnabled,
                                          title, kHWMonitorKeyTitle,
                                          value, kHWMonitorKeyValue,
                                          key, kHWMonitorKeyKey,
                                          [NSNumber numberWithLong:mainGroup], kHWMonitorKeyGroup,
                                          nil]];
            
            if (_globalColorIndex + 1 >= [_globalColors count])
                _globalColorIndex = 0;
            else
                _globalColorIndex++;
        }
    }
}

- (void)recieveItems:(NSNotification*)aNotification
{
    [_sensorsController removeAllItems];
    
    [_graphsController removeObjectsAtArrangedObjectIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, [[_graphsController arrangedObjects] count])]];
    
    [_sensorsController setFirstFavoriteItem:GetLocalizedString(@"Menu bar items:") firstAvailableItem:GetLocalizedString(@"Available items:")];
    
    if ([aNotification object] && [aNotification userInfo]) {
        NSString *favoritesString = (NSString*)[aNotification object];
        NSArray *favoritesList = [favoritesString length] > 0 ? [favoritesString componentsSeparatedByString:@","] : [NSArray array];
        NSDictionary *sensorsList = [aNotification userInfo];
        
        _globalColors = [[NSMutableArray alloc] init];
        
        NSColorList *colorList = [NSColorList colorListNamed:@"Crayons"];
        
        for (NSUInteger i = 8; i < [[colorList allKeys] count]; i++) {
            NSString *key = [[colorList allKeys] objectAtIndex:i];
            NSColor *color = [colorList colorWithKey:key];
            double intensity = (color.redComponent + color.blueComponent + color.greenComponent) / 3.0;
            double red = [color redComponent];
            double green = [color greenComponent];
            double blue = [color blueComponent];
            BOOL blackAndWhite = red == green && red == blue && green == blue;
            
            if (intensity >= 0.333 && intensity <=0.90 && !blackAndWhite)
                [_globalColors addObject:color];
        }

        _globalColorIndex = 0;
        
        // Add thermometer icon
        HWMonitorIcon *thermometer = [self getIconByName:kHWMonitorIconThermometer];
        [_sensorsController addItem:GetLocalizedString([thermometer name]) icon:[thermometer image] key:[thermometer name]];
        
        // Add hwmonitor icon
        HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconDefault];
        [_sensorsController addItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
        // Add device icon
        icon = [self getIconByName:kHWMonitorIconDevice];
        [_sensorsController addItem:GetLocalizedString([icon name]) icon:[icon image] key:[icon name]];
        
        // Add all sensors
        [self addItemsFromDictionary:sensorsList inGroup:kHWSensorGroupTemperature];
        [self addItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupTemperature];
        [self addItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupRemainingLife];
        [self addItemsFromDictionary:sensorsList inGroup:kSMARTSensorGroupRemainingBlocks];
        [self addItemsFromDictionary:sensorsList inGroup:kHWSensorGroupMultiplier];
        [self addItemsFromDictionary:sensorsList inGroup:kHWSensorGroupFrequency];
        [self addItemsFromDictionary:sensorsList inGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer];
        [self addItemsFromDictionary:sensorsList inGroup:kHWSensorGroupVoltage];

        // Setup favorites
        if ([favoritesList count] == 0)
            [_sensorsController setFavoritesItemsFromArray:[NSArray arrayWithObjects:[thermometer name], nil]];
        else
            [_sensorsController setFavoritesItemsFromArray:favoritesList];
        
        [self setUserInterfaceEnabled:[[NSObject alloc] init]];
        
        // Set active app status
        [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorAppIsActive object:HWMonitorBooleanYES userInfo:nil deliverImmediately:YES];
        
        [_toggleMenuButton setState:YES];
    }
    else {
        [self setUserInterfaceEnabled:nil];
    }
}

-(void)valuesChanged:(NSNotification *)aNotification
{
    if ([aNotification userInfo]) {
        NSDictionary *list = [aNotification userInfo];
        
        for (NSMutableDictionary *item in [_sensorsController arrangedObjects]) {
            
            NSString *key = [item objectForKey:kHWMonitorKeyKey];
            
            if ([[list allKeys] containsObject:key])
                [item setValue:[[list objectForKey:key] objectForKey:kHWMonitorKeyValue] forKey:kHWMonitorKeyValue];
        }
        
        for (NSMutableDictionary *item in [_graphsController arrangedObjects]) {
            
            NSString *key = [item objectForKey:kHWMonitorKeyKey];
            
            if ([[list allKeys] containsObject:key])
                [item setValue:[[list objectForKey:key] objectForKey:kHWMonitorKeyValue] forKey:kHWMonitorKeyValue];
        }
        
        [_temperatureGraph captureDataToHistoryFromDictionary:list];
        [_frequencyGraph captureDataToHistoryFromDictionary:list];
        [_tachometerGraph captureDataToHistoryFromDictionary:list];
        [_voltageGraph captureDataToHistoryFromDictionary:list];
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
        
        CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
        
        int count = 0;
        
        do {
            count++;
            sleep(1);
            error = CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
        } while ((!menuExtra || error) && count < 3);
        
        system("killall SystemUIServer");
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
    NSArray *favorites = [_sensorsController getFavoritesItems];
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    for (NSDictionary *item in favorites)
        [list addObject:[item valueForKey:@"Key"]];
    
    NSString *favoritesList = [list componentsJoinedByString:@","];
    
    NSMutableDictionary *info = [[NSMutableDictionary alloc] init];
    
    for (NSDictionary *item in [_sensorsController getAllItems]) {
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
    
    BOOL useFahrenheit = [matrix selectedRow] == 1;
    
    [_temperatureGraph setUseFahrenheit:useFahrenheit];
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseFahrenheitChanged object:useFahrenheit ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)useBigFontChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseBigFontChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)useShadowEffectChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseShadowsChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)useBSDNamesChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUseBSDNamesChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(IBAction)showVolumeNamesChanged:(id)sender
{
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorShowVolumeNamesChanged object:[sender state] ? HWMonitorBooleanYES : HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
}

-(float)getSmcSensorsUpdateRate
{
    float value = [_sensorsUpdateRateSlider floatValue];
    
    [_sensorsUpdateRateText setStringValue:[NSString stringWithFormat:@"%1.1f %@", value, GetLocalizedString(@"sec")]];
    
    return value;
}

-(float)getSmartSensorsUpdateRate
{
    float value = [_SMARTUpdateRateSlider floatValue];
    
    [_SMARTUpdateRateText setStringValue:[NSString stringWithFormat:@"%1.0f %@", value, GetLocalizedString(@"min")]];
    
    return value;
}

-(void)updateRateChanged:(id)sender
{
    NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:
                          [NSNumber numberWithFloat:[self getSmcSensorsUpdateRate]], kHWMonitorSmcSensorsUpdateRate,
                          [NSNumber numberWithFloat:[self getSmartSensorsUpdateRate]], kHWMonitorSmartSensorsUpdateRate,
                          nil];
    
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorUpdateRateChanged object:nil userInfo:info deliverImmediately:YES];
}

- (IBAction)graphsTableViewClicked:(id)sender
{
    [_temperatureGraph setNeedsDisplay:YES];
    [_frequencyGraph setNeedsDisplay:YES];
    [_tachometerGraph setNeedsDisplay:YES];
    [_voltageGraph setNeedsDisplay:YES];
}

- (NSView*)viewForTag:(NSUInteger)tag
{
    switch (tag) {
        case 0:
            return _preferencesView;
            
        case 1:
            return _favoritesView;
            
        case 2:
            return _graphsView;
            
        default:
            return _preferencesView;
    }
}

- (NSRect)screenFrameForView:(NSView*)view
{
    NSWindow *window = _window;
    NSRect newFrameRect = [window frameRectForContentRect:[view frame]];
    NSRect oldFrameRect = [window frame];
    NSSize newSize = newFrameRect.size;
    NSSize oldSize = oldFrameRect.size;
    
    NSRect frame = [window frame];
    frame.size = newSize;
    frame.origin.y -= newSize.height - oldSize.height;
    frame.origin.x -= (newSize.width - oldSize.width) / 2;
    
    return frame;
}

- (void)setViewForTag:(NSInteger)tag
{
    NSView *newView = [self viewForTag:tag];
    
    for (NSToolbarItem *item in [_prefsToolbar items]) {
        if ([item tag] == tag)
            [item setImage:[[self getIconByName:[item itemIdentifier]] alternateImage]];
        else if ([item tag] == _lastSelectedView)
            [item setImage:[[self getIconByName:[item itemIdentifier]] image]];
    }
    
    if ([_window contentView] == newView)
        return;
    
    _lastSelectedView = tag;
    
    NSRect newFrame = [self screenFrameForView:newView];
    
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:0.2];
    
    [_preferencesView setAlphaValue:0.0];
    [_graphsView setAlphaValue:0.0];
    [_favoritesView setAlphaValue:0.0];
    
    [_window setContentView:newView];
    [_window setFrame:newFrame display:YES animate:YES];
    
    if ([_window isVisible])
        [[newView animator] setAlphaValue:1.0];
    else
        [newView setAlphaValue:1.0];
    
    [[_window standardWindowButton:NSWindowZoomButton] setEnabled:NO];
    
    if (newView == _graphsView) {
        //[[_window standardWindowButton:NSWindowZoomButton] setEnabled:YES];
        [_window setContentMaxSize:NSMakeSize(MAXFLOAT, MAXFLOAT)];
        [_window setContentMinSize:NSMakeSize(700, 600)];
    }
    else if (newView == _favoritesView) {
        [_window setContentMaxSize:NSMakeSize(MAXFLOAT, MAXFLOAT)];
        [_window setContentMinSize:NSMakeSize(360, 600)];
    }
    else
    {
        [_window setContentMinSize:[newView frame].size];
        [_window setContentMaxSize:[newView frame].size];
    }
    
    [NSAnimationContext endGrouping];
    
    [_defaults setInteger:tag forKey:kHWMonitorSelectedTag];
}

- (void)saveViewSizeForTag:(NSInteger)tag
{
    NSView *view = [self viewForTag:tag];
    
    [_defaults setFloat:[view frame].size.width forKey:[NSString stringWithFormat:@"PrefsView%ldWidth", tag]];
    [_defaults setFloat:[view frame].size.height forKey:[NSString stringWithFormat:@"PrefsView%ldHeight", tag]];
}

- (void)loadViewSizeForTag:(NSInteger)tag
{
    NSView *view = [self viewForTag:tag];
    
    CGFloat width = [_defaults floatForKey:[NSString stringWithFormat:@"PrefsView%ldWidth", tag]];
    CGFloat height = [_defaults floatForKey:[NSString stringWithFormat:@"PrefsView%ldHeight", tag]];

    if (height > 0 && width > 0)
        [view setFrameSize:NSMakeSize(width, height)];
}

-(void)prefsToolbarClicked:(id)sender
{
    [self setViewForTag:[sender tag]];
}

-(void)localizeObject:(id)view
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
    else if ([view isKindOfClass:[NSTabView class]]) {
        for (NSTabViewItem *item in [(NSTabView*)view tabViewItems]) {
            [item setLabel:GetLocalizedString([item label])];
            [self localizeObject:[item view]];
        }
    }
    else if ([view isKindOfClass:[NSToolbar class]]) {
        for (NSToolbarItem *item in [(NSToolbar*)view items]) {
            [item setLabel:GetLocalizedString([item label])];
            [self localizeObject:[item view]];
        }
        
        return;
    }
    else if ([view isKindOfClass:[NSMenu class]]) {
        NSMenu *menu = (NSMenu*)view;
        
        [menu setTitle:GetLocalizedString([menu title])];
        
        for (id subItem in [menu itemArray]) {
            if ([subItem isKindOfClass:[NSMenuItem class]]) {
                NSMenuItem* menuItem = subItem;
                
                [menuItem setTitle:GetLocalizedString([menuItem title])];
                
                if ([menuItem hasSubmenu])
                    [self localizeObject:[menuItem submenu]];
            }
        }
        
        return;
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
    
    if ([view isKindOfClass:[NSView class]] )
        for(NSView *subView in [view subviews])
            [self localizeObject:subView];
}

- (void)checkConnectionWithMenuExtra
{
    if (!_menuExtraConnectionActive) {
        
        void *menuExtra = nil;
        
        int error = CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
        
        if (!error && menuExtra) {
            // Set active app status
            [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorAppIsActive object:HWMonitorBooleanYES userInfo:nil deliverImmediately:YES];
        }
    }
    else [self performSelector:@selector(checkConnectionWithMenuExtra) withObject:nil afterDelay:5.0];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    _defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];
    
    // setup views
    //[_preferencesView setAlphaValue:0.0];
    //[_menubarView setAlphaValue:0.0];
    //[_graphsView setAlphaValue:0.0];
    for (int i = 0; i < 3; i++)
        [[self viewForTag:i] setAlphaValue:0.0];
    
    for (int i = 1; i < 3; i++)
        [self loadViewSizeForTag:i];
    
    // Load toolbar images
    for (NSToolbarItem *item in [_prefsToolbar items]) {
        NSString *name = [item itemIdentifier];
        [self addIconNamed:name FromImage:[item image]];
    }
    
    //[_prefsToolbar setSelectedItemIdentifier:[_defaults stringForKey:kHWMonitorSelectedTag]];
    _lastSelectedView = [_defaults integerForKey:kHWMonitorSelectedTag];
    [self setViewForTag:_lastSelectedView];
    
    [_window setTitle:GetLocalizedString([_window title])];
    [self localizeObject:_prefsToolbar];
    [self localizeObject:_preferencesView];
    [self localizeObject:_favoritesView];
    [self localizeObject:_graphsView];
    [self localizeObject:_menu];
    
    [self getSmcSensorsUpdateRate];
    [self getSmartSensorsUpdateRate];
            
    [_temperatureGraph setGroup:kHWSensorGroupTemperature | kSMARTSensorGroupTemperature];
    [_frequencyGraph setGroup:kHWSensorGroupFrequency];
    [_tachometerGraph setGroup:kHWSensorGroupTachometer];
    [_voltageGraph setGroup:kHWSensorGroupVoltage];
    
    void *menuExtra = nil;
    
    int error = CoreMenuExtraGetMenuExtra((__bridge CFStringRef)@"org.hwsensors.HWMonitorExtra", &menuExtra);
    
    [_toggleMenuButton setState:!error && menuExtra];
        
    [_temperatureGraph setUseFahrenheit:[_defaults boolForKey:kHWMonitorUseFahrenheitKey]];
    
    // undocumented call
    [[NSUserDefaultsController sharedUserDefaultsController] _setDefaults:_defaults];
    
    [self loadIconNamed:kHWMonitorIconDefault AsTemplate:NO];
    [self loadIconNamed:kHWMonitorIconThermometer AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconDevice AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconTemperatures AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconHddTemperatures AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconSsdLife AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconMultipliers AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconFrequencies AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconTachometers AsTemplate:YES];
    [self loadIconNamed:kHWMonitorIconVoltages AsTemplate:YES];
    
    // Set custom cursor on items table
    //[_itemsScrollView setDocumentCursor:[NSCursor openHandCursor]];
    
    // Update version label
	NSString *version = [[NSString alloc] initWithFormat:@"v%@", [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"]];
    
    //[_versionLabel setTitleWithMnemonic:version];
    [_versionLabel setStringValue:version];
    
    [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(recieveItems:) name:HWMonitorRecieveItems object: NULL];
    [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(valuesChanged:) name:HWMonitorValuesChanged object: NULL];
    
    // Request items
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorRequestItems object:nil userInfo:nil deliverImmediately:YES];
    
    [self performSelector:@selector(checkConnectionWithMenuExtra) withObject:nil afterDelay:0.0];
    
    [_window setIsVisible:YES];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    for (int i = 1; i < 3; i++)
        [self saveViewSizeForTag:i];
    
    // Set active app status
    [[NSDistributedNotificationCenter defaultCenter] postNotificationName:HWMonitorAppIsActive object:HWMonitorBooleanNO userInfo:nil deliverImmediately:YES];
    
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
}

-(BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    return NO;
}

@end
