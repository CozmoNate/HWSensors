//
//  GraphsController.m
//  HWMonitor
//
//  Created by kozlek on 24.02.13.
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

#import "GraphsController.h"

#import "HWMonitorSensor.h"
#import "HWMonitorItem.h"
#import "HWMonitorGroup.h"
#import "GraphsView.h"
#import "SensorCell.h"

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

@implementation GraphsController

-(void)setUseFahrenheit:(BOOL)useFahrenheit
{
    _useFahrenheit = useFahrenheit;
    
    [_temperatureGraph setUseFahrenheit:useFahrenheit];
    [_frequencyGraph setUseFahrenheit:useFahrenheit];
    [_tachometerGraph setUseFahrenheit:useFahrenheit];
    [_voltageGraph setUseFahrenheit:useFahrenheit];
}

-(void)setUseSmoothing:(BOOL)useSmoothing
{
    _useSmoothing = useSmoothing;
    
    [_temperatureGraph setUseSmoothing:useSmoothing];
    [_frequencyGraph setUseSmoothing:useSmoothing];
    [_tachometerGraph setUseSmoothing:useSmoothing];
    [_voltageGraph setUseSmoothing:useSmoothing];
}

-(NSArray *)colorsList
{
    return _colorsList;
}

-(HWMonitorItem *)selectedItem
{
    if ([_graphsTableView selectedRow] >= 0 && [_graphsTableView selectedRow] < [_items count]) {
        return [_items objectAtIndex:[_graphsTableView selectedRow]];
    }
    
    return nil;
}

-(id)init
{
    self = [super init];
    
    if (self) {
        _colorsList = [[NSMutableArray alloc] init];
        
        NSColorList *list = [NSColorList colorListNamed:@"Crayons"];
        
        for (NSUInteger i = 8; i < [[list allKeys] count]; i++) {
            NSString *key = [[list allKeys] objectAtIndex:i];
            NSColor *color = [list colorWithKey:key];
            double intensity = (color.redComponent + color.blueComponent + color.greenComponent) / 3.0;
            double red = [color redComponent];
            double green = [color greenComponent];
            double blue = [color blueComponent];
            BOOL blackAndWhite = red == green && red == blue && green == blue;
            
            if (intensity >= 0.330 && intensity <=0.90 && !blackAndWhite)
                [_colorsList addObject:color];
        }
    }
    
    return self;
}

-(void)setupWithGroups:(NSArray *)groups
{
    if (!_items) {
        _items = [[NSMutableArray alloc] init];
    }
    else {
        [_items removeAllObjects];
    }
    
    if (!_hiddenItems) {
        _hiddenItems = [[NSMutableArray alloc] init];
    }
    else {
        [_hiddenItems removeAllObjects];
    }
    
    [_items addObject:@"TEMPERATURES"];
    [_items addObjectsFromArray:[_temperatureGraph addItemsForSensorGroup:kHWSensorGroupTemperature | kSMARTSensorGroupTemperature fromGroupsList:groups]];
    [_items addObject:@"FREQUENCIES"];
    [_items addObjectsFromArray:[_frequencyGraph addItemsForSensorGroup:kHWSensorGroupFrequency fromGroupsList:groups]];
    [_items addObject:@"FANS"];
    [_items addObjectsFromArray:[_tachometerGraph addItemsForSensorGroup:kHWSensorGroupTachometer fromGroupsList:groups]];
    [_items addObject:@"VOLTAGES"];
    [_items addObjectsFromArray:[_voltageGraph addItemsForSensorGroup:kHWSensorGroupVoltage fromGroupsList:groups]];
}

- (void) captureDataToHistoryNow
{
    for (NSUInteger index = 0; index < [_items count]; index++) {
        
        id item = [_items objectAtIndex:index];
        
        if ([item isKindOfClass:[HWMonitorItem class]]) {
            SensorCell *cell = [_graphsTableView viewAtColumn:0 row:index makeIfNecessary:NO];

            [[cell valueField] setStringValue:[[item sensor] formattedValue]];
        }
    }
    
    [_temperatureGraph captureDataToHistoryNow];
    [_frequencyGraph captureDataToHistoryNow];
    [_tachometerGraph captureDataToHistoryNow];
    [_voltageGraph captureDataToHistoryNow];
}

- (BOOL) checkItemIsHidden:(HWMonitorItem*)item
{
    return [_hiddenItems indexOfObject:item] != NSNotFound;
}

// Events

-(IBAction)graphsTableViewClicked:(id)sender
{
    [_temperatureGraph setNeedsDisplay:YES];
    [_frequencyGraph setNeedsDisplay:YES];
    [_tachometerGraph setNeedsDisplay:YES];
    [_voltageGraph setNeedsDisplay:YES];
}

-(IBAction)graphsCheckButtonClicked:(id)sender
{
    if ([sender tag] >= 0 && [sender tag] < [_items count]) {
        HWMonitorItem *item = [_items objectAtIndex:[sender tag]];
        
        if ([sender state] == NSOnState) {
            [_hiddenItems removeObject:item];
        }
        else {
            [_hiddenItems addObject:item];
        }
    }
    
    [_temperatureGraph calculateGraphBoundsFindExtremes:YES];
    [_frequencyGraph calculateGraphBoundsFindExtremes:YES];
    [_tachometerGraph calculateGraphBoundsFindExtremes:YES];
    [_voltageGraph calculateGraphBoundsFindExtremes:YES];
    
    [self graphsTableViewClicked:sender];
}

// NSTableView delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [_items count];
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 18;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return ![[_items objectAtIndex:row] isKindOfClass:[NSString class]];
}

/*-(BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    return [[_items objectAtIndex:row] isKindOfClass:[NSString class]];
}*/

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id item = [_items objectAtIndex:row];
    
    if ([item isKindOfClass:[NSString class]]) {
        NSTableCellView *groupCell = [tableView makeViewWithIdentifier:item owner:self];
        
        [[groupCell textField] setStringValue:GetLocalizedString(item)];
        
        return groupCell;
    }
    else if ([item isKindOfClass:[HWMonitorItem class]]) {
        SensorCell *sensorCell = [tableView makeViewWithIdentifier:@"Sensor" owner:self];
        
        HWMonitorSensor *sensor = [item sensor];
        
        [[sensorCell textField] setStringValue:GetLocalizedString([sensor title])];
        [[sensorCell valueField] setStringValue:[sensor formattedValue]];
        [[sensorCell colorWell] setColor:[item color]];
        [[sensorCell checkBox] setState:YES];
        [[sensorCell checkBox] setTag:[_items indexOfObject:item]];
        
        [sensorCell setRepresentedObject:item];
        
        return sensorCell;
    }
    
    return nil;
}

@end
