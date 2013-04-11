//
//  AppDelegate.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import "AppController.h"
#import "HWMonitorDefinitions.h"
#import "HWMonitorEngine.h"
#import "HWMonitorGroup.h"

#import "GroupCell.h"
#import "PrefsSensorCell.h"

#import "Localizer.h"

@implementation AppController

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [Localizer localizeView:self.window];
    [Localizer localizeView:_graphsController.window];
    
    _defaults = [[BundleUserDefaults alloc] initWithPersistentDomainName:@"org.hwsensors.HWMonitor"];

    // Call undocumented function
    [[NSUserDefaultsController sharedUserDefaultsController] _setDefaults:_defaults];
    
    [self loadIconNamed:kHWMonitorIconDefault];
    [self loadIconNamed:kHWMonitorIconThermometer];
    [self loadIconNamed:kHWMonitorIconDevice];
    [self loadIconNamed:kHWMonitorIconTemperatures];
    [self loadIconNamed:kHWMonitorIconHddTemperatures];
    [self loadIconNamed:kHWMonitorIconSsdLife];
    [self loadIconNamed:kHWMonitorIconMultipliers];
    [self loadIconNamed:kHWMonitorIconFrequencies];
    [self loadIconNamed:kHWMonitorIconTachometers];
    [self loadIconNamed:kHWMonitorIconVoltages];
    [self loadIconNamed:kHWMonitorIconBattery];
    
    _colorThemes = [ColorTheme createColorThemes];
    
    _engine = [[HWMonitorEngine alloc] initWithBundle:[NSBundle mainBundle]];
    
    [_engine setUseFahrenheit:[_defaults boolForKey:kHWMonitorUseFahrenheitKey]];
    [_engine setUseBsdNames:[_defaults boolForKey:kHWMonitorUseBSDNames]];
    
    [[_popupController statusItemView] setEngine:_engine];
    [[_popupController statusItemView] setUseBigFont:[_defaults boolForKey:kHWMonitorUseBigStatusMenuFont]];
    [[_popupController statusItemView] setUseShadowEffect:[_defaults boolForKey:kHWMonitorUseShadowEffect]];
    [_popupController setShowVolumeNames:[_defaults integerForKey:kHWMonitorShowVolumeNames]];
    [_popupController setColorTheme:[_colorThemes objectAtIndex:[_defaults integerForKey:kHWMonitorColorThemeIndex]]];
    
    [_graphsController setUseFahrenheit:[_engine useFahrenheit]];
    [_graphsController setUseSmoothing:[_defaults boolForKey:kHWMonitorGraphsUseDataSmoothing]];
    [_graphsController setBackgroundMonitoring:[_defaults boolForKey:kHWMonitorGraphsBackgroundMonitor]];
    [_graphsController setIsTopmost:[_defaults boolForKey:kHWMonitorWindowTopmost]];
    
    [_favoritesTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
    [_favoritesTableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];
    [_sensorsTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
    [_sensorsTableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
    
    [self updateRateChanged:nil];
    
    //[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(drivesChanged:) name:NSWorkspaceDidMountNotification object:nil];
	//[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(drivesChanged:) name:NSWorkspaceDidUnmountNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(wakeFromSleep:) name:NSWorkspaceDidWakeNotification object:nil];
    
    [self performSelector:@selector(rebuildSensorsList) withObject:nil afterDelay:0.0];
    
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(updateLoop)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateLoop)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:0.05 invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //[[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver: self name: NSWorkspaceDidMountNotification object:nil];
	//[[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver: self name: NSWorkspaceDidUnmountNotification object:nil];
}

-(void)wakeFromSleep:(id)sender
{
    
}

-(void)showWindow:(id)sender
{
    for (HWMonitorSensor *sensor in [_engine sensors]) {
        id cell = [_sensorsTableView viewAtColumn:0 row:[self getIndexOfItem:[sensor name]] makeIfNecessary:NO];
        
        if (cell && [cell isKindOfClass:[PrefsSensorCell class]]) {
            [[cell valueField] takeStringValueFrom:sensor];
        }
    }
    
    [super showWindow:sender];
}

- (void)loadIconNamed:(NSString*)name
{
    if (!_icons)
        _icons = [[NSMutableDictionary alloc] init];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:name ofType:@"png"]];
    
    [image setTemplate:YES];
    
    NSImage *altImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[name stringByAppendingString:@"_template"] ofType:@"png"]];

    [altImage setTemplate:YES];
    
    [_icons setObject:[HWMonitorIcon iconWithName:name image:image alternateImage:altImage] forKey:name];
}

- (HWMonitorIcon*)getIconByName:(NSString*)name
{
    return [_icons objectForKey:name];
}

- (HWMonitorIcon*)getIconByGroup:(NSUInteger)group
{
    if ((group & kHWSensorGroupTemperature) || (group & kSMARTGroupTemperature)) {
        return [self getIconByName:kHWMonitorIconTemperatures];
    }
    else if ((group & kSMARTGroupRemainingLife) || (group & kSMARTGroupRemainingBlocks)) {
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
    else if (group & (kHWSensorGroupVoltage | kHWSensorGroupCurrent | kHWSensorGroupPower)) {
        return [self getIconByName:kHWMonitorIconVoltages];
    }
    else if (group & kBluetoothGroupBattery) {
        return [self getIconByName:kHWMonitorIconBattery];
    }
    
    return nil;
}

- (void)addItem:(id)item forKey:(NSString*)key
{
    if (![_items objectForKey:key]) {
        [_items setObject:item forKey:key];
        [_ordering addObject:key];
    }
}

- (id)getItemAtIndex:(NSUInteger)index
{
    return [_items objectForKey:[_ordering objectAtIndex:index]];
}

- (NSUInteger)getIndexOfItem:(NSString*)key
{
    return [_ordering indexOfObject:key];
}

- (void)updateSmartSensors;
{
    NSArray *sensors = [_engine updateSmartSensors];
    [self updateValuesForSensors:sensors];
}

- (void)updateSmcSensors
{
    NSArray *sensors = [_engine updateSensors];
    [self updateValuesForSensors:sensors];
}

- (void)updateFavoritesSensors
{
    NSArray *sensors = [_engine updateSensorsList:_favorites];
    [self updateValuesForSensors:sensors];
}

- (void)updateValuesForSensors:(NSArray*)sensors
{
    if ([self.window isVisible]) {
        for (HWMonitorSensor *sensor in sensors) {
            id cell = [_sensorsTableView viewAtColumn:0 row:[self getIndexOfItem:[sensor name]] makeIfNecessary:NO];
            
            if (cell && [cell isKindOfClass:[PrefsSensorCell class]]) {
                [[cell valueField] takeStringValueFrom:sensor];
            }
        }
        
        [_favorites enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            id cell = [_favoritesTableView viewAtColumn:0 row:idx + 1 makeIfNecessary:NO];
            
            if (cell && [cell isKindOfClass:[PrefsSensorCell class]]) {
                [[cell valueField] takeStringValueFrom:obj];
            }
        }];
    }
    
    [_popupController updateValuesForSensors:sensors];
    [_graphsController captureDataToHistoryNow];
}

- (BOOL)updateLoop
{
    if (_scheduleRebuildSensors) {
        [self rebuildSensorsList];
        _scheduleRebuildSensors = FALSE;
    }
    else {
        NSDate *now = [NSDate dateWithTimeIntervalSinceNow:0.0];
        
        if ([self.window isVisible] || [_popupController.window isVisible] || [_graphsController.window isVisible] || [_graphsController backgroundMonitoring]) {
            if ([_smcSensorsLastUpdated timeIntervalSinceNow] < (- _smcSensorsUpdateInterval)) {
                [self performSelectorInBackground:@selector(updateSmcSensors) withObject:nil];
                _smcSensorsLastUpdated = now;
                return TRUE;
            }
        }
        else if ([_favoritesSensorsLastUpdated timeIntervalSinceNow] < (- _smcSensorsUpdateInterval)) {
            [self performSelectorInBackground:@selector(updateFavoritesSensors) withObject:nil];
            _favoritesSensorsLastUpdated = now;
            return TRUE;
        }
    
        if ([_smartSensorsLastUpdated timeIntervalSinceNow] < (- _smartSensorsUpdateInterval)) {
            [self performSelectorInBackground:@selector(updateSmartSensors) withObject:nil];
            _smartSensorsLastUpdated = now;
            return TRUE;
        }
    }
    
    return FALSE;
}

- (void)rebuildSensorsTableView
{
    if (!_ordering)
        _ordering = [[NSMutableArray alloc] init];
    else
        [_ordering removeAllObjects];
    
    if (!_items)
        _items = [[NSMutableDictionary alloc] init];
    else
        [_items removeAllObjects];
 
    // Add icons
    [self addItem:@"Icons" forKey:@"Icons"];
    
    HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconDefault]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconThermometer]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconDevice]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconTemperatures]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconHddTemperatures]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconSsdLife]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconMultipliers]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconFrequencies]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconTachometers]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconVoltages]; [self addItem:icon forKey:icon.name];
    icon = [self getIconByName:kHWMonitorIconBattery]; [self addItem:icon forKey:icon.name];

    // Add sensors
    //[self addItem:@"Sensors" forKey:@"Sensors"];
    
    for (HWMonitorGroup *group in _groups) {
        if ([[group items] count]) {
            [self addItem:[group title] forKey:[group title]];
        }
        
        for (HWMonitorItem *item in [group items]) {
            [self addItem:item forKey:item.sensor.name];
        }
    }
    
    if ([_favorites count] == 0) {
        [_favorites addObject:[self getIconByName:kHWMonitorIconThermometer]];
    }
    
    [_favoritesTableView reloadData];
    [_sensorsTableView reloadData];
}

- (void)rebuildSensorsList
{    
    if (!_favorites) {
        _favorites = [[NSMutableArray alloc] init];
    }
    else {
        [_favorites removeAllObjects];
    }
    
    if (!_groups)
        _groups = [[NSMutableArray alloc] init];
    else
        [_groups removeAllObjects];
    
    [_engine rebuildSensorsList];
    
    if ([[_engine sensors] count] > 0) {
        
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kHWSensorGroupTemperature title:GetLocalizedString(@"TEMPERATURES") image:[self getIconByName:kHWMonitorIconTemperatures]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kSMARTGroupTemperature title:GetLocalizedString(@"DRIVE TEMPERATURES") image:[self getIconByName:kHWMonitorIconHddTemperatures]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kSMARTGroupRemainingLife title:GetLocalizedString(@"SSD REMAINING LIFE") image:[self getIconByName:kHWMonitorIconSsdLife]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kSMARTGroupRemainingBlocks title:GetLocalizedString(@"SSD REMAINING BLOCKS") image:[self getIconByName:kHWMonitorIconSsdLife]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kHWSensorGroupMultiplier | kHWSensorGroupFrequency title:GetLocalizedString(@"FREQUENCIES") image:[self getIconByName:kHWMonitorIconFrequencies]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kHWSensorGroupPWM |kHWSensorGroupTachometer title:GetLocalizedString(@"FANS") image:[self getIconByName:kHWMonitorIconTachometers]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kHWSensorGroupVoltage title:GetLocalizedString(@"VOLTAGES") image:[self getIconByName:kHWMonitorIconVoltages]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kHWSensorGroupCurrent title:GetLocalizedString(@"CURRENTS") image:[self getIconByName:kHWMonitorIconVoltages]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kHWSensorGroupPower title:GetLocalizedString(@"POWERS") image:[self getIconByName:kHWMonitorIconVoltages]]];
        [_groups addObject:[HWMonitorGroup groupWithEngine:_engine sensorGroup:kBluetoothGroupBattery title:GetLocalizedString(@"BATTERIES") image:[self getIconByName:kHWMonitorIconBattery]]];
        
        [_favorites removeAllObjects];
        
        NSArray *favoritesList = [_defaults objectForKey:kHWMonitorFavoritesList];
        
        if (favoritesList) {
            
            NSUInteger i = 0;
            
            for (i = 0; i < [favoritesList count]; i++) {
                
                NSString *name = [favoritesList objectAtIndex:i];
                
                HWMonitorSensor *sensor = nil;
                HWMonitorIcon *icon = nil;
                
                if ((sensor = [[_engine keys] objectForKey:name])) {
                    [_favorites addObject:sensor];
                }
                else if ((icon = [_icons objectForKey:name])) {
                    [_favorites addObject:icon];
                }
            }
        }
        
        NSArray *hiddenList = [_defaults objectForKey:kHWMonitorHiddenList];
        
        for (NSString *key in hiddenList) {
            if ([[[_engine keys] allKeys] containsObject:key]) {
                
                HWMonitorSensor *sensor = [[_engine keys] objectForKey:key];
                
                if (sensor)
                    [[sensor representedObject] setVisible:NO];
            }
        }
    
    }
    
    [_popupController setupWithGroups:_groups];
    [_popupController.statusItemView setFavorites:_favorites];
    
    [_graphsController setupWithGroups:_groups];
    
    [self rebuildSensorsTableView];
}

- (IBAction)toggleSensorVisibility:(id)sender
{
    id item = [self getItemAtIndex:[sender tag]];
    
    [item setVisible:[sender state]];
    
    [_popupController setupWithGroups:_groups];
    
    NSMutableArray *hiddenList = [[NSMutableArray alloc] init];
    
    for (id item in [_items allValues]) {
        if ([item isKindOfClass:[HWMonitorItem class]] && ![item isVisible]) {
            [hiddenList addObject:[[item sensor] name]];
        }
    }
    
    [_defaults setObject:hiddenList forKey:kHWMonitorHiddenList];
    
    [_defaults synchronize];
}

#pragma mark Events

-(IBAction)favoritesChanged:(id)sender
{
    [_favoritesTableView reloadData];
    
    [_popupController.statusItemView setFavorites:_favorites];
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    for (id item in _favorites) {
        NSString *name = nil;
        
        if ([item isKindOfClass:[HWMonitorIcon class]] || [item isKindOfClass:[HWMonitorSensor class]]) {
            name = [item name];
        }
        else continue;
        
        if ([[_engine keys] objectForKey:name] || [_icons objectForKey:name]) {
            [list addObject:name];
        }
    }
    
    [_defaults setObject:list forKey:kHWMonitorFavoritesList];
}

-(IBAction)useFahrenheitChanged:(id)sender
{
    BOOL useFahrenheit = [sender selectedRow] == 1;
    
    [_engine setUseFahrenheit:useFahrenheit];
    
    [_sensorsTableView reloadData];
    [_popupController reloadData];
    [_graphsController setUseFahrenheit:useFahrenheit];
    
    [_defaults synchronize];
}

- (IBAction)colorThemeChanged:(id)sender
{
    [_popupController setColorTheme:[_colorThemes objectAtIndex:[sender selectedRow]]];
}

-(IBAction)useBigFontChanged:(id)sender
{
    [_popupController.statusItemView setUseBigFont:[sender state]];
    [_defaults synchronize];
}

-(IBAction)useShadowEffectChanged:(id)sender
{
    [_popupController.statusItemView setUseShadowEffect:[sender state]];
    [_defaults synchronize];
}

-(IBAction)useBSDNamesChanged:(id)sender
{
    [_engine setUseBsdNames:[sender state]];
    [_popupController.tableView reloadData];
    [self rebuildSensorsTableView];
    [_defaults synchronize];
}

-(IBAction)showVolumeNamesChanged:(id)sender
{
    [_popupController setShowVolumeNames:[sender state]];
    [_popupController reloadData];
    [self rebuildSensorsTableView];
    [_defaults synchronize];
}

-(float)getSmcSensorsUpdateRate
{
    [_defaults synchronize];
    
    float value = [_defaults floatForKey:kHWMonitorSmcSensorsUpdateRate];
    float validatedValue = value > 10 ? 10 : value < 1 ? 1 : value;
    
    if (value != validatedValue) {
        value = validatedValue;
        [_defaults setFloat:value forKey:kHWMonitorSmcSensorsUpdateRate];
    }
    
    [_smcUpdateRateTextField setStringValue:[NSString stringWithFormat:@"%1.1f %@", value, GetLocalizedString(@"sec")]];
    
    return value;
}

-(float)getSmartSensorsUpdateRate
{
    [_defaults synchronize];
    
    float value = [_defaults floatForKey:kHWMonitorSmartSensorsUpdateRate];
    float validatedValue = value > 30 ? 30 : value < 5 ? 5 : value;
    
    if (value != validatedValue) {
        value = validatedValue;
        [_defaults setFloat:value forKey:kHWMonitorSmartSensorsUpdateRate];
    }
    
    [_smartUpdateRateTextField setStringValue:[NSString stringWithFormat:@"%1.0f %@", value, GetLocalizedString(@"min")]];
    
    return value;
}

-(void)updateRateChanged:(NSNotification *)aNotification
{
    _smcSensorsUpdateInterval = [self getSmcSensorsUpdateRate];
    _smcSensorsLastUpdated = [NSDate dateWithTimeIntervalSince1970:0.0];
    _favoritesSensorsLastUpdated = _smcSensorsLastUpdated;
     _smartSensorsUpdateInterval = [self getSmartSensorsUpdateRate] * 60;
    _smartSensorsLastUpdated = [NSDate dateWithTimeIntervalSince1970:0.0];
}

- (IBAction)toggleGraphSmoothing:(id)sender
{
    [_graphsController setUseSmoothing:[sender state] == NSOnState];
    [_defaults synchronize];
}

-(IBAction)graphsBackgroundMonitorChanged:(id)sender
{
    [_graphsController setBackgroundMonitoring:[sender state]];
    [_defaults synchronize];
}

-(IBAction)graphsWindowTopmostChanged:(id)sender
{
    [_graphsController setIsTopmost:[sender state]];
    [_defaults synchronize];
}

#pragma mark PopupControllerDelegate

- (void) popupWillOpen:(id)sender
{
    [self updateLoop];
}

#pragma mark  NSTableViewDelegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (tableView == _favoritesTableView) {
        return [_favorites count] + 1;
    }

    return [_items count];
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 19;
}

-(BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    return [[self getItemAtIndex:row] isKindOfClass:[NSString class]];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if (tableView == _favoritesTableView) {
        if (row == 0) {
            GroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];
            
            [[groupCell textField] setStringValue:GetLocalizedString(@"Menubar items")];
            
            //[groupCell setColorTheme:[_colorThemes objectAtIndex:0]];
            
            return groupCell;
        }
        else {
            id item = [_favorites objectAtIndex:row - 1];
            
            if ([item isKindOfClass:[HWMonitorSensor class]]) {
                HWMonitorSensor *sensor = (HWMonitorSensor*)item;
                
                PrefsSensorCell *itemCell = [tableView makeViewWithIdentifier:@"Sensor" owner:self];;
                
                [itemCell.imageView setImage:[[self getIconByGroup:[sensor group]] image]];
                [itemCell.textField setStringValue:[sensor title]];
                [itemCell.valueField setStringValue:[sensor stringValue]];
                
                return itemCell;
            }
            else if ([item isKindOfClass:[HWMonitorIcon class]]) {
                PrefsSensorCell *iconCell = [tableView makeViewWithIdentifier:@"Icon" owner:self];
                
                [[iconCell imageView] setObjectValue:[item image]];
                [[iconCell textField] setStringValue:GetLocalizedString([item name])];
                
                return iconCell;
            }
        }
    }
    else if (tableView == _sensorsTableView) {
        id item = [self getItemAtIndex:row];
        
        if ([item isKindOfClass:[HWMonitorItem class]]) {
            HWMonitorSensor *sensor = [item sensor];
            
            PrefsSensorCell *itemCell = [tableView makeViewWithIdentifier:@"Sensor" owner:self];
            
            [itemCell.checkBox setState:[item isVisible]];
            //[itemCell.checkBox setToolTip:GetLocalizedString(@"Show sensor in HWMonitor menu")];
            [itemCell.checkBox setTag:[_ordering indexOfObject:[sensor name]]];
            [itemCell.imageView setImage:[[self getIconByGroup:[sensor group]] image]];
            [itemCell.textField setStringValue:[sensor title]];
            [itemCell.valueField setStringValue:[sensor stringValue]];
            
            return itemCell;
        }
        else if ([item isKindOfClass:[HWMonitorIcon class]]) {
            PrefsSensorCell *iconCell = [tableView makeViewWithIdentifier:@"Icon" owner:self];
            
            [[iconCell imageView] setObjectValue:[item image]];
            [[iconCell textField] setStringValue:GetLocalizedString([item name])];
            
            return iconCell;
        }
        else if ([item isKindOfClass:[NSString class]]) {
            GroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];
            
            [[groupCell textField] setStringValue:GetLocalizedString(item)];
            //[groupCell setColorTheme:[_colorThemes objectAtIndex:0]];
            
            return groupCell;
        }
    }

    return nil;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    if (tableView == _favoritesTableView) {
        if ([rowIndexes firstIndex] == 0) {
            return NO;
        }
        
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorTableViewDataType];
        
        _hasDraggedFavoriteItem = YES;
    }
    else if (tableView == _sensorsTableView) {
        id item = [self getItemAtIndex:[rowIndexes firstIndex]];
        
        if ([item isKindOfClass:[NSString class]]) {
            return NO;
        }
//        else if ([item isKindOfClass:[HWMonitorItem class]] && [_favorites containsObject:[item sensor]]) {
//            //return NO;
//            _currentItemDragOperation = NSDragOperationPrivate;
//        }
        
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, NSStringPboardType, nil] owner:self];
        
        [pboard setData:indexData forType:kHWMonitorTableViewDataType];
        [pboard setString:[_ordering objectAtIndex:[rowIndexes firstIndex]] forType:NSStringPboardType];
        
        _hasDraggedFavoriteItem = NO;
    }
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)toRow proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    //_currentItemDragOperation = NSDragOperationNone;
    
    if (tableView == _favoritesTableView) {
        [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
        
        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];
        
        if ([info draggingSource] == _favoritesTableView) {
            _currentItemDragOperation = toRow < 1 || toRow == fromRow || toRow == fromRow + 1 ? NSDragOperationNone : NSDragOperationMove;
        }
        else if ([info draggingSource] == _sensorsTableView) {
            id item = [self getItemAtIndex:fromRow];
            
            if ([item isKindOfClass:[HWMonitorItem class]]) {
                _currentItemDragOperation = [_favorites containsObject:[item sensor]] ? NSDragOperationPrivate : toRow > 0  ? NSDragOperationCopy : NSDragOperationNone;
            }
            else _currentItemDragOperation = toRow > 0 ? NSDragOperationCopy : NSDragOperationNone;
        }
    }
    else if (tableView == _sensorsTableView) {
        _currentItemDragOperation = NSDragOperationNone;
    }
    
    return _currentItemDragOperation;
}

-(void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    if (tableView == _favoritesTableView && (operation == NSDragOperationDelete || _currentItemDragOperation == NSDragOperationDelete))
    {
        NSPasteboard* pboard = [session draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];

        [_favorites removeObjectAtIndex:[rowIndexes firstIndex] - 1];
        
        [self favoritesChanged:tableView];
        
        NSShowAnimationEffect(NSAnimationEffectPoof, screenPoint, NSZeroSize, nil, nil, nil);
    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (tableView == _favoritesTableView) {
        
        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];
        
        if ([info draggingSource] == _sensorsTableView) {
            id item = [self getItemAtIndex:fromRow];
            
            if ([item isKindOfClass:[HWMonitorItem class]]) {
                if (![_favorites containsObject:[item sensor]]) {
                    [_favorites insertObject:[item sensor] atIndex:toRow > 0 ? toRow - 1 : 0];
                }
            }
            else if ([item isKindOfClass:[HWMonitorIcon class]]) {
                [_favorites insertObject:item  atIndex:toRow > 0 ? toRow - 1 : 0];
            }
        }
        else if ([info draggingSource] == _favoritesTableView) {
            id item = [_favorites objectAtIndex:fromRow - 1];
            
            [_favorites insertObject:item atIndex:toRow > 0 ? toRow - 1 : 0];
            [_favorites removeObjectAtIndex:fromRow > toRow ? fromRow : fromRow - 1];
        }
    
        [self favoritesChanged:tableView];
    }
    
    return YES;
}

@end
