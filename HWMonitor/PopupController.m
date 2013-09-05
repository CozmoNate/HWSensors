//
//  PopupController.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
// 

#import "PopupController.h"

#import "Localizer.h"

#import "HWMonitorDefinitions.h"
#import "HWMonitorGroup.h"
#import "GroupCell.h"
#import "SensorCell.h"
#import "BatteryCell.h"
#import "UpdatesController.h"

@implementation PopupController

@synthesize statusItem = _statusItem;
@synthesize statusItemView = _statusItemView;

-(void)setColorTheme:(ColorTheme *)colorTheme
{
    _colorTheme = colorTheme;
    
    [(OBMenuBarWindow*)self.window setColorTheme:colorTheme];
    [_tableView reloadData];
}

- (id)init
{
    self = [super initWithWindowNibName:@"PopupController"];
    
    if (self != nil)
    {
        _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        
        _statusItemView = [[StatusItemView alloc] initWithFrame:NSMakeRect(0, 0, 22, 22) statusItem:_statusItem];
        
        _statusItemView.image = [NSImage imageNamed:@"thermometer"];
        _statusItemView.alternateImage = [NSImage imageNamed:@"thermometer_template"];
        
        [_statusItemView setAction:@selector(togglePanel:)];
        [_statusItemView setTarget:self];
        
        [self performSelector:@selector(initialSetup) withObject:nil afterDelay:0.0];
    }
    
    return self;
}

-(void)awakeFromNib
{
    [[self statusItemView] setUseBigFont:[[NSUserDefaults standardUserDefaults] boolForKey:kHWMonitorUseBigStatusMenuFont]];
    [[self statusItemView] setUseShadowEffect:[[NSUserDefaults standardUserDefaults] boolForKey:kHWMonitorUseShadowEffect]];
    [self setShowVolumeNames:[[NSUserDefaults standardUserDefaults] integerForKey:kHWMonitorShowVolumeNames]];
}

- (void)dealloc
{
    [[NSStatusBar systemStatusBar] removeStatusItem:_statusItem];
}

-(void)showWindow:(id)sender
{
    if (self.window.isVisible)
        return;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupWillOpen:)]) {
        [self.delegate popupWillOpen:self];
    }
    
    // Update values
    for (id item in _items) {
        if ([item isKindOfClass:[HWMonitorItem class]]) {
            [self updateValueForItem:item];
        }
    }

    [self resizeToContentAndOrderFront:YES];
    
//    if (!_windowFilter) {
//        _windowFilter = [[WindowFilter alloc] initWithWindow:self.window name:@"CIGaussianBlur" andOptions:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.5] forKey:@"inputRadius"]];
//    }
//    else {
//        [_windowFilter setFilterOptions:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.5] forKey:@"inputRadius"]];
//    }
    
    self.statusItemView.isHighlighted = YES;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupDidOpen:)]) {
        [self.delegate popupDidOpen:self];
    }
}

-(void)close
{
    if (!self.window.isVisible)
        return;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupWillClose:)]) {
        [self.delegate popupWillClose:self];
    }
    
//    if (_windowFilter) {
//        [_windowFilter setFilterOptions:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:0.0] forKey:@"inputRadius"]];
//    }

//    [NSAnimationContext beginGrouping];
//    [[NSAnimationContext currentContext] setDuration:CLOSE_DURATION];
//    [self.window.animator setAlphaValue:0];
//    [NSAnimationContext endGrouping];
    
    //dispatch_after(dispatch_walltime(NULL, NSEC_PER_SEC * CLOSE_DURATION * 2), dispatch_get_main_queue(), ^{
        [self.window orderOut:nil];
    //});
    
    self.statusItemView.isHighlighted = NO;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupDidClose:)]) {
        [self.delegate popupDidClose:self];
    }
}
#pragma mark -
#pragma mark Events



#pragma mark -
#pragma mark Actions

- (void)togglePanel:(id)sender
{
    OBMenuBarWindow* menubarWindow = (OBMenuBarWindow*)self.window;
    
    if (menubarWindow)
    {
        if (menubarWindow.isVisible && (menubarWindow.isKeyWindow || menubarWindow.attachedToMenuBar))
        {
            [self close];
            self.statusItemView.isHighlighted = NO;
        }
        else
        {
            if (!menubarWindow.attachedToMenuBar) {
                [NSApp activateIgnoringOtherApps:YES];
            }
            
            [self showWindow:nil];
            self.statusItemView.isHighlighted = YES;
        }
    }
}

- (void)showAboutPanel:(id)sender
{
    [_aboutController showWindow:sender];
}

- (void)openPreferences:(id)sender
{
    [_appController showWindow:sender];
}

- (void)showGraphsWindow:(id)sender
{
    [_graphsController showWindow:sender];
}

- (void)checkForUpdates:(id)sender
{
    [self performSelectorInBackground:@selector(checkForUpdatesDialog) withObject:nil];
}

#pragma mark -
#pragma mark Methods

- (void)initialSetup
{
    [_tableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorPopupItemDataType]];
    [_tableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
    
    //[[_titleField cell] setBackgroundStyle:NSBackgroundStyleRaised];
    
    // Install status item into the menu bar
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;
    
    menubarWindow.statusItemView = _statusItemView;
    menubarWindow.statusItem = _statusItem;
    menubarWindow.attachedToMenuBar = YES;
    menubarWindow.hideWindowControls = YES;
    
    menubarWindow.toolbarView = _toolbarView;
    
    [menubarWindow setWorksWhenModal:YES];
    
    [Localizer localizeView:menubarWindow];
    [Localizer localizeView:_toolbarView];
    
    // Make main menu font size smaller
    NSFont* font = [NSFont menuFontOfSize:13];
	NSDictionary* fontAttribute = [NSDictionary dictionaryWithObjectsAndKeys:font, NSFontAttributeName, nil];
    
    for (id subItem in [_mainMenu itemArray]) {
        if ([subItem isKindOfClass:[NSMenuItem class]]) {
            NSMenuItem* menuItem = subItem;
            NSString* title = [menuItem title];
            
            NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:title attributes:fontAttribute];
            
            [menuItem setAttributedTitle:attributedTitle];
        }
    }
    
    [self resizeToContentAndOrderFront:NO];
}

- (void)checkForUpdatesDialog
{
    UpdatesController *controller = (UpdatesController*)self.updatesController;
    
    [controller checkForUpdatesForced];
}

- (void) setupWithGroups:(NSArray*)groups
{
    _items = [[NSMutableArray alloc] init];
    
    // Add special toolbar item
    //[_items addObject:@"Toolbar"];
    
    if ([groups count] > 0) {
        
        // Restore indexes
        NSMutableDictionary *itemIndex = [[NSMutableDictionary alloc] initWithDictionary:[[NSUserDefaults standardUserDefaults] objectForKey:kHWMonitorItemIndex]];
        
        for (HWMonitorGroup *group in groups) {
            if ([group checkVisibility]) {
                [_items addObject:group];

                // Sort out
                [[group items] sortUsingComparator:^NSComparisonResult(id obj1, id obj2) {
                    NSNumber *idx1 = [itemIndex objectForKey:[[obj1 sensor] name]];
                    NSNumber *idx2 = [itemIndex objectForKey:[[obj2 sensor] name]];
                    
                    return idx1 && idx2 ? [idx1 integerValue] - [idx2 integerValue] : 0;
                }];
                
                // Add items
                for (HWMonitorItem *item in [group items]) {
                    if ([item isVisible]) {
                        [_items addObject:item];
                        [itemIndex setObject:[NSNumber numberWithInteger:[_items count] - 1] forKey:item.sensor.name];
                    }
                }
            }
        }
        
        // Save key index
        [[NSUserDefaults standardUserDefaults] setObject:itemIndex forKey:kHWMonitorItemIndex];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
    else {
        [_items addObject:@"Dummy"];
    }
    
    [self reloadData];
}

- (void)resizeToContentAndOrderFront:(BOOL)orderFront
{
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;
    
    CGFloat height = 13; // ??
    
    for (int i = 0; i < [_items count]; i++) {
        height += [self tableView:_tableView heightOfRow:i];
    }
    
    height = 6 + (menubarWindow.attachedToMenuBar ? OBMenuBarWindowArrowHeight : 0) + (height ? height : _tableView.frame.size.height);

    [menubarWindow setContentSize:NSMakeSize(menubarWindow.frame.size.width, height)];
    [menubarWindow setMaxSize:NSMakeSize(menubarWindow.maxSize.width, menubarWindow.frame.size.height)];
    
    // Order front if needed
    if (orderFront) {
        [self.window makeKeyAndOrderFront:self];
    }
}

- (void)reloadData
{
    [_tableView reloadData];
    [self resizeToContentAndOrderFront:NO];
    [_statusItemView setNeedsDisplay:YES];
}

-(void)updateValueForItem:(HWMonitorItem*)item
{
    if ([item isVisible]) {
        id cell = [_tableView viewAtColumn:0 row:[_items indexOfObject:item] makeIfNecessary:NO];
        
        if (cell) {
            NSColor *valueColor;
            
            switch ([item.sensor level]) {
                    /*case kHWSensorLevelDisabled:
                     break;
                     
                     case kHWSensorLevelNormal:
                     break;*/
                    
                case kHWSensorLevelModerate:
                    valueColor = [NSColor colorWithCalibratedRed:0.7f green:0.3f blue:0.03f alpha:1.0f];
                    break;
                    
                case kHWSensorLevelExceeded:
                    [[cell textField] setTextColor:[NSColor redColor]];
                case kHWSensorLevelHigh:
                    valueColor = [NSColor redColor];
                    break;
                    
                default:
                    valueColor = _colorTheme.itemValueTitleColor;
                    break;
            }
            
            [[cell valueField] takeStringValueFrom:item.sensor];
            
            if (![[[cell valueField] textColor] isEqualTo:valueColor]) {
                [[cell valueField] setTextColor:valueColor];
            }
            
            if ([item.sensor genericDevice] && [[item.sensor genericDevice] isKindOfClass:[GenericBatteryDevice class]]) {
                [cell setGaugeLevel:[item.sensor intValue]];
            }
        }
    }
}

-(void)updateValuesForSensors:(NSArray *)sensors
{
    if ([self.window isVisible]) {
        for (HWMonitorSensor *sensor in sensors) {
            [self updateValueForItem:[sensor representedObject]];
        }
    }
    
    [_statusItemView setNeedsDisplay:YES];
}

#pragma mark -
#pragma mark NSTableView delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [_items count];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{    
    id item = [_items objectAtIndex:row];
    
    if ([item isKindOfClass:[HWMonitorGroup class]]) {
        return 19;
    }
    else if ([item isKindOfClass:[HWMonitorItem class]]) {
        HWMonitorSensor *sensor = [item sensor];
        
        if ((_showVolumeNames && [sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[ATAGenericDrive class]]) ||
            ([sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[GenericBatteryDevice class]] && [[sensor genericDevice] productName])) {
            return 27;
        }
        else {
            return 17;
        }
    }
//    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Toolbar"]) {
//        return kHWMonitorToolbarHeight;
//    }


    return 17;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

//- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
//{
//    id item = [_items objectAtIndex:row];
//    
//    if ([item isKindOfClass:[HWMonitorGroup class]]) {
//        return _hasScroller;
//    }
//    
//    return NO;
//}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    if (self.tableView != tableView) {
        return NO;
    }
    
    id item = [_items objectAtIndex:[rowIndexes firstIndex]];
    
    if ([item isKindOfClass:[HWMonitorItem class]]) {
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorPopupItemDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorPopupItemDataType];

        return YES;
    }

    return NO;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)toRow proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (_tableView != tableView || [info draggingSource] != _tableView) {
        return NO;
    }
    
    [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
    
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorPopupItemDataType];
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    NSInteger fromRow = [rowIndexes firstIndex];
    
    id sourceItem = [_items objectAtIndex:fromRow];
    
    _currentItemDragOperation = NSDragOperationNone;
    
    if (toRow > 0) {
        
        _currentItemDragOperation = NSDragOperationMove;
        
        if (toRow < [_items count]) {
            
            if (toRow == fromRow || toRow == fromRow + 1) {
                _currentItemDragOperation = NSDragOperationNone;
            }
            else {
                id destinationItem = [_items objectAtIndex:toRow];
                
                if ([destinationItem isKindOfClass:[HWMonitorItem class]] && [(HWMonitorItem*)sourceItem group] != [(HWMonitorItem*)destinationItem group]) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
                else {
                    destinationItem = [_items objectAtIndex:toRow - 1];
                    
                    if ([destinationItem isKindOfClass:[HWMonitorItem class]] && [(HWMonitorItem*)sourceItem group] != [(HWMonitorItem*)destinationItem group]) {
                        _currentItemDragOperation = NSDragOperationNone;
                    }
                }
            }
        }
        else {
            id destinationItem = [_items objectAtIndex:toRow - 1];
            
            if ([destinationItem isKindOfClass:[HWMonitorItem class]] && [(HWMonitorItem*)sourceItem group] != [(HWMonitorItem*)destinationItem group]) {
                _currentItemDragOperation = NSDragOperationNone;
            }
        }
    }
    
    return _currentItemDragOperation;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (self.tableView != tableView) {
        return NO;
    }
    
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorPopupItemDataType];
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    NSInteger fromRow = [rowIndexes firstIndex];

    HWMonitorItem *sourceItem = [_items objectAtIndex:fromRow];
    //HWMonitorItem *destinationItem = [_items objectAtIndex:toRow];
    
    [_items insertObject:sourceItem atIndex:toRow];
    [_items removeObjectAtIndex:toRow < fromRow ? fromRow + 1 : fromRow];
    
    [_tableView reloadData];
    
    // Rebuild and save key index
    NSMutableDictionary *itemIndex = [[NSMutableDictionary alloc] init];
    
    [_items enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if ([obj isKindOfClass:[HWMonitorItem class]]) {
            [itemIndex setObject:[NSNumber numberWithInteger:idx] forKey:[[obj sensor] name]];
        }
    }];
    
    [[NSUserDefaults standardUserDefaults] setObject:itemIndex forKey:kHWMonitorItemIndex];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    return YES;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id item = [_items objectAtIndex:row];
    
    if ([item isKindOfClass:[HWMonitorGroup class]]) {
        HWMonitorGroup *group = item;
        
        GroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];
        
        [groupCell setColorTheme:_colorTheme];
        [groupCell.textField setStringValue:[group title]];
        [groupCell.imageView setObjectValue:_colorTheme.useDarkIcons ? [[group icon] image] : [[group icon] alternateImage]];
        
        return groupCell;
    }
    else if ([item isKindOfClass:[HWMonitorItem class]]) {
        HWMonitorSensor *sensor = [item sensor];
        
        id cell = [tableView makeViewWithIdentifier:[item representation] owner:self];
        
        [cell setColorTheme:_colorTheme];
        
        if (_showVolumeNames && [sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[ATAGenericDrive class]]) {
            [[cell subtitleField] setStringValue:[[sensor genericDevice] volumesNames]];
            [[cell subtitleField] setHidden:NO];
        }
        else if ([sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[GenericBatteryDevice class]]) {
            if ([[sensor genericDevice] productName]) {
                [[cell subtitleField] setStringValue:[[sensor genericDevice] productName]];
                [[cell subtitleField] setHidden:NO];
            }
            else  {
                [[cell subtitleField] setHidden:YES];
            }
            
            [cell setGaugeLevel:[sensor intValue]];
        }
        else {
            [[cell subtitleField] setHidden:YES];
        }
        
        [[cell textField] setStringValue:[sensor title]];
        //[[cell valueField] setStringValue:[sensor stringValue]];
        [[cell valueField] takeStringValueFrom:sensor];
        
        return cell;
    }
//    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Toolbar"]) {
//        NSTableCellView *buttonsCell = [tableView makeViewWithIdentifier:item owner:self];
//        
//        [buttonsCell.textField setTextColor:_colorTheme.toolbarTitleColor];
//        
//        return buttonsCell;
//    }
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Dummy"]) {
        NSTableCellView *dummyCell = [tableView makeViewWithIdentifier:item owner:self];
        
        [dummyCell.textField setTextColor:_colorTheme.itemTitleColor];
        
        return dummyCell;
    }
    
    return nil;
}

@end
