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
#import "SensorCell.h"

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

#define AddItem(item, name) \
[_items setObject:item forKey:name]; \
[_ordering addObject:name]; \
[_indexes setObject:[NSNumber numberWithUnsignedInteger:[_ordering indexOfObject:name]] forKey:name];

#define GetItemAtIndex(index) \
[_items objectForKey:[_ordering objectAtIndex:index]]

#define GetIndexOfItem(name) \
[_ordering indexOfObject:name]

#define SetItemAsFavorite(name) \
[_ordering removeObject:name]; \
[_ordering insertObject:name atIndex:[_ordering indexOfObject:_availableGroupItem]]; \

@implementation AppController

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
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
    
    _colorThemes = [ColorTheme createColorThemes];
    
    [_popupController setColorTheme:[_colorThemes objectAtIndex:[_defaults integerForKey:kHWMonitorColorThemeIndex]]];
    
    [self updateRateChanged:nil];
    
    [_sensorsTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
    [_sensorsTableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:YES];
    
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:@selector(updateLoop)]];
    [invocation setTarget:self];
    [invocation setSelector:@selector(updateLoop)];
    
    [[NSRunLoop mainRunLoop] addTimer:[NSTimer timerWithTimeInterval:0.05 invocation:invocation repeats:YES] forMode:NSRunLoopCommonModes];
    
    [self performSelector:@selector(rebuildSensorsList) withObject:nil afterDelay:0.0];
    
    //[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(drivesChanged:) name:NSWorkspaceDidMountNotification object:nil];
	//[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(drivesChanged:) name:NSWorkspaceDidUnmountNotification object:nil];
}

-(void)dealloc
{
    //[[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver: self name: NSWorkspaceDidMountNotification object:nil];
	//[[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver: self name: NSWorkspaceDidUnmountNotification object:nil];
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
    else if (group & kHWSensorGroupVoltage) {
        return [self getIconByName:kHWMonitorIconVoltages];
    }
    
    return nil;
}

- (void)updateSmartSensors;
{
    NSArray *updatedSensors = [_engine updateSmartSensors];
    [self updateValuesForSensors:updatedSensors];
}

- (void)updateSmcSensors
{
    NSArray *updatedSensors = [_engine updateSmcSensors];
    [self updateValuesForSensors:updatedSensors];
}

- (void)updateFavoritesSensors
{
    NSArray *updatedSensors = [_engine updateSmcSensorsList:_favorites];
    [self updateValuesForSensors:updatedSensors];
}

- (void)captureDataToHistory
{
    [_graphsController captureDataToHistoryNow];
}

- (void)updateValuesForSensors:(NSArray*)sensors
{
    //[_sensorsTableView reloadData];
    
    for (HWMonitorSensor *sensor in sensors) {
        NSUInteger index = GetIndexOfItem([sensor name]);
        
        if (index >= [_sensorsTableView numberOfRows])
            continue;
        
        id cell = [_sensorsTableView viewAtColumn:0 row:index makeIfNecessary:NO];
        
        if (cell && [cell isKindOfClass:[SensorCell class]]) {
            [[cell valueField] setStringValue:[sensor formattedValue]];
        }
    }
    
    [_popupController.statusItemView setNeedsDisplay:YES];
    
    if ([_popupController.window isVisible]) {
        [_popupController updateValues];
    }
    
    if ([_graphsController.window isVisible]) {
        [_graphsController captureDataToHistoryNow];
    }
}

- (void)updateLoop
{   
    if (_scheduleRebuildSensors) {
        [self rebuildSensorsList];
        _scheduleRebuildSensors = FALSE;
    }
    else {
        NSDate *now = [NSDate dateWithTimeIntervalSinceNow:0.0];
        
        if ([self.window isVisible] || [_popupController.window isVisible] || [_graphsController.window isVisible]) {
            if ([_smcSensorsLastUpdated timeIntervalSinceNow] < (- _smcSensorsUpdateInterval)) {
                [self performSelectorInBackground:@selector(updateSmcSensors) withObject:nil];
                _smcSensorsLastUpdated = now;
            }
        }
        else if ([_favoritesSensorsLastUpdated timeIntervalSinceNow] < (- _smcSensorsUpdateInterval)) {
            [self performSelectorInBackground:@selector(updateFavoritesSensors) withObject:nil];
            _favoritesSensorsLastUpdated = now;
        }
    
        if ([_smartSensorsLastUpdated timeIntervalSinceNow] < (- _smartSensorsUpdateInterval)) {
            [self performSelectorInBackground:@selector(updateSmartSensors) withObject:nil];
            _smartSensorsLastUpdated = now;
        }
    }
}

/*- (void) togglePopupPanel:(id)sender
{
    [_popupController setHasActivePanel:![_popupController hasActivePanel]];
    //NSLog(@"Toggle popup panel");
}*/

- (void)rebuildSensorsTableView
{
    if (!_ordering)
        _ordering = [[NSMutableArray alloc] init];
    else
        [_ordering removeAllObjects];
    
    if (!_indexes)
        _indexes = [[NSMutableDictionary alloc] init];
    else
        [_indexes removeAllObjects];
    
    if (!_items)
        _items = [[NSMutableDictionary alloc] init];
    else
        [_items removeAllObjects];

    // Add groups
    _favoriteGroupItem = @"Menubar items";
    AddItem(_favoriteGroupItem, _favoriteGroupItem)
    _availableGroupItem = @"Icons";
    AddItem(_availableGroupItem, _availableGroupItem);
 
    // Add icons
    HWMonitorIcon *icon = [self getIconByName:kHWMonitorIconDefault]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconThermometer]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconDevice]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconTemperatures]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconHddTemperatures]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconSsdLife]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconMultipliers]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconFrequencies]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconTachometers]; AddItem(icon, icon.name);
    icon = [self getIconByName:kHWMonitorIconVoltages]; AddItem(icon, icon.name);

    // Add sensors
    AddItem(@"Sensors", @"Sensors");
    
    for (HWMonitorGroup *group in _groups) {
        /*if ([group checkVisibility]) {
            AddItem([group title], [group title]);
        }*/
        
        for (HWMonitorItem *item in [group items]) {
            AddItem(item, item.sensor.name);
        }
    }
    
    if ([_favorites count] == 0) {
        SetItemAsFavorite(kHWMonitorIconThermometer);
    }
    else {
        for (id item in _favorites) {            
            NSString *name = nil;
            
            if ([item isKindOfClass:[HWMonitorIcon class]] || [item isKindOfClass:[HWMonitorSensor class]]) {
                name = [item name];
            }
            else continue;
            
            if ([[_engine keys] objectForKey:name] || [_icons objectForKey:name]) {
                SetItemAsFavorite(name);
            }
        }
    }
    
    [_sensorsTableView reloadData];
}

- (void)rebuildSensorsList
{
    if (!_engine) {
        _engine = [[HWMonitorEngine alloc] initWithBundle:[NSBundle mainBundle]];
        [[_popupController statusItemView] setEngine:_engine];
    }
    
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
       
    [_engine setUseFahrenheit:[_defaults boolForKey:kHWMonitorUseFahrenheitKey]];
    [_engine setUseBSDNames:[_defaults boolForKey:kHWMonitorUseBSDNames]];
    
    [[_popupController statusItemView] setUseBigFont:[_defaults boolForKey:kHWMonitorUseBigStatusMenuFont]];
    [[_popupController statusItemView] setUseShadowEffect:[_defaults boolForKey:kHWMonitorUseShadowEffect]];
    
    [_popupController setShowVolumeNames:[_defaults integerForKey:kHWMonitorShowVolumeNames]];
    
    [_graphsController setUseSmoothing:[_defaults boolForKey:kHWMonitorGraphsUseDataSmoothing]];
    
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
    id item = GetItemAtIndex([sender tag]);
    
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

-(IBAction)favoritesChanged:(id)sender
{
    [_sensorsTableView reloadData];
    
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
    [_graphsController setUseFahrenheit:useFahrenheit];
    [_popupController.statusItemView setNeedsDisplay:YES];
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
    [_engine setUseBSDNames:[sender state]];
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

// PopupController delegate

- (void) popupPanelShouldOpen:(id)sender
{
    [self updateLoop];
}

// NSTableView delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [_items count];
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 20;
}

-(BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    return [GetItemAtIndex(row) isKindOfClass:[NSString class]];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id item = GetItemAtIndex(row);
    
    if ([item isKindOfClass:[HWMonitorItem class]]) {
        HWMonitorSensor *sensor = [item sensor];
        
        SensorCell *sensorCell = [tableView makeViewWithIdentifier:@"Sensor" owner:self];
        
        [sensorCell.checkBox setState:[item isVisible]];
        [sensorCell.checkBox setToolTip:GetLocalizedString(@"Show sensor in HWMonitor menu")];
        [sensorCell.checkBox setTag:[_ordering indexOfObject:[sensor name]]];
        [sensorCell.imageView setImage:[[self getIconByGroup:[sensor group]] image]];
        [sensorCell.textField setStringValue:[sensor title]];
        [sensorCell.valueField setStringValue:[sensor formattedValue]];
        
        return sensorCell;
    }
    else if ([item isKindOfClass:[HWMonitorIcon class]]) {
        NSTableCellView *iconCell = [tableView makeViewWithIdentifier:@"Icon" owner:self];
        
        [[iconCell imageView] setObjectValue:[item image]];
        [[iconCell textField] setStringValue:GetLocalizedString([item name])];
        
        return iconCell;
    }
    else if ([item isKindOfClass:[NSString class]]) {
        NSTableCellView *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];
        
        [[groupCell textField] setStringValue:GetLocalizedString(item)];
        
        return groupCell;
    }

    return nil;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return false;
}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    id item = GetItemAtIndex([rowIndexes firstIndex]);
    
    if ([item isKindOfClass:[NSString class]]) {
        return NO;
    }
    
    NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
    
    [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, NSStringPboardType, nil] owner:self];
    [pboard setData:indexData forType:kHWMonitorTableViewDataType];
    
    [pboard setString:[_ordering objectAtIndex:[rowIndexes firstIndex]] forType:NSStringPboardType];
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
    
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    
    NSInteger itemsRow = [_ordering indexOfObject:_availableGroupItem];
    NSInteger fromRow = [rowIndexes firstIndex];
    
    return (fromRow > itemsRow && row > 0 && row <= itemsRow) || (fromRow < itemsRow && row > 0)  ? NSDragOperationMove : NSDragOperationNone;
    
    return NSDragOperationMove;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
    
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    
    NSInteger itemsRow = GetIndexOfItem(_availableGroupItem);
    NSInteger fromRow = [rowIndexes firstIndex];
    
    NSString *movingItemName = [_ordering objectAtIndex:fromRow];
    NSInteger movingItemIndex = [[_indexes objectForKey:movingItemName] integerValue];
    
    if (fromRow < itemsRow && toRow > itemsRow) {
        
        NSInteger index;
        
        for (index = itemsRow + 1; index < [_items count]; index++) {
            
            NSString *itemName = [_ordering objectAtIndex:index];
            NSInteger itemIndex = [[_indexes objectForKey:itemName] integerValue];
            
            if (itemIndex > movingItemIndex) {
                [_ordering insertObject:movingItemName atIndex:index];
                [_ordering removeObjectAtIndex:fromRow > index ? fromRow + 1 : fromRow];
                break;
            }
        }
        
        if (index >= [_items count]) {
            [_ordering insertObject:movingItemName atIndex:index];
            [_ordering removeObjectAtIndex:fromRow];
        }
    }
    else {
        [_ordering insertObject:movingItemName atIndex:toRow];
        [_ordering removeObjectAtIndex:fromRow > toRow ? fromRow + 1 : fromRow];
    }
    
    // Renew favorites list
    itemsRow = GetIndexOfItem(_availableGroupItem);
    
    [_favorites removeAllObjects];
    
    for (NSUInteger index = 0; index < itemsRow; index++) {
        id item = GetItemAtIndex(index);
        
        if ([item isKindOfClass:[HWMonitorItem class]]) {
            [_favorites addObject:[item sensor]];
        }
        else if ([item isKindOfClass:[HWMonitorIcon class]]) {
            [_favorites addObject:item];
        }
    }
    
    [self favoritesChanged:tableView];
    
    return YES;
}



@end
