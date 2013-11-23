//
//  PopupController.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
// 

#import "PopupController.h"

#import "Localizer.h"

#import "HWMonitorDefinitions.h"

#import "PopupGroupCell.h"
#import "PopupSensorCell.h"
#import "PopupAtaSmartSensorCell.h"
#import "PopupBatteryCell.h"

#import "JLNFadingScrollView.h"

#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMGroup.h"
#import "HWMSensor.h"

@implementation PopupController

@synthesize statusItem = _statusItem;
@synthesize statusItemView = _statusItemView;
@synthesize toolbarView = _toolbarView;

#pragma mark -
#pragma mark Properties

-(BOOL)hasDraggedFavoriteItem
{
    return YES;
}

#pragma mark -
#pragma mark Overridden Methods

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

        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self initialSetup];
        }];
    }
    
    return self;
}

- (void)dealloc
{
    [[NSStatusBar systemStatusBar] removeStatusItem:_statusItem];
}

-(void)showWindow:(id)sender
{
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    if (menubarWindow.isVisible)
        return;

    if (self.delegate && [self.delegate respondsToSelector:@selector(popupWillOpen:)]) {
        [self.delegate popupWillOpen:self];
    }

    [self layoutContent:NO orderFront:YES];
    
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
#pragma mark Methods

- (void)initialSetup
{
    //[[_titleField cell] setBackgroundStyle:NSBackgroundStyleRaised];

    // Install status item into the menu bar
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    menubarWindow.statusItemView = _statusItemView;
    menubarWindow.statusItem = _statusItem;
    menubarWindow.attachedToMenuBar = YES;
    menubarWindow.hideWindowControls = YES;

    menubarWindow.toolbarView = _toolbarView;

    [menubarWindow setWorksWhenModal:YES];

    //    [Localizer localizeView:menubarWindow];
    //    [Localizer localizeView:_toolbarView];

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

    [self layoutContent:YES orderFront:NO];

    [(OBMenuBarWindow*)self.window setColorTheme:self.monitorEngine.configuration.colorTheme];
    [(JLNFadingScrollView *)_scrollView setFadeColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];

    [_tableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorPopupItemDataType]];
    [_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];

    [Localizer localizeView:self.window];
    [Localizer localizeView:_toolbarView];

    [self addObserver:self forKeyPath:@"monitorEngine.configuration.colorTheme" options:NSKeyValueObservingOptionNew context:nil];
    [self addObserver:self forKeyPath:@"monitorEngine.arrangedItems" options:NSKeyValueObservingOptionNew context:nil];

    [_statusItemView setMonitorEngine:_monitorEngine];
}

- (void)layoutContent:(BOOL)resizeToContent orderFront:(BOOL)orderFront
{
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    if (resizeToContent) {
        CGFloat height = menubarWindow.toolbarHeight + 6;

        for (int i = 0; i < self.monitorEngine.arrangedItems.count; i++) {
            height += [self tableView:_tableView heightOfRow:i];
        }

        height = height > menubarWindow.screen.visibleFrame.size.height ? menubarWindow.screen.visibleFrame.size.height - menubarWindow.toolbarHeight : height;

        [menubarWindow setFrame:NSMakeRect(menubarWindow.frame.origin.x,
                                           menubarWindow.frame.origin.y + (menubarWindow.frame.size.height - height),
                                           menubarWindow.frame.size.width,
                                           height) display:YES animate:NO];
    }

    // Order front if needed
    if (orderFront) {
        [menubarWindow makeKeyAndOrderFront:self];
    }
}

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

#pragma mark -
#pragma mark Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"monitorEngine.configuration.colorTheme"]) {
        [(OBMenuBarWindow*)self.window setColorTheme:self.monitorEngine.configuration.colorTheme];
        [(JLNFadingScrollView *)_scrollView setFadeColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];

        [_tableView reloadData];
    }
    else if ([keyPath isEqual:@"monitorEngine.arrangedItems"]) {

        [_tableView reloadData];
        [self layoutContent:YES orderFront:NO];
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{

}

-(void)awakeFromNib
{

}

- (void)windowDidAttachToStatusBar:(id)sender
{
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    [menubarWindow setMaxSize:NSMakeSize(menubarWindow.maxSize.width, menubarWindow.frame.size.height)];
    [menubarWindow setMinSize:NSMakeSize(menubarWindow.minSize.width, menubarWindow.toolbarHeight + 6)];

    //[NSApp deactivate];
}

- (void)windowDidDetachFromStatusBar:(id)sender
{
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    [menubarWindow setMaxSize:NSMakeSize(menubarWindow.maxSize.width, menubarWindow.frame.size.height)];
    [menubarWindow setMinSize:NSMakeSize(menubarWindow.minSize.width, menubarWindow.toolbarHeight + 6)];

    if (menubarWindow.isKeyWindow) {
        [NSApp activateIgnoringOtherApps:YES];
    }
}

- (void)windowDidBecomeKey:(id)sender
{
    for (id subveiw in _toolbarView.subviews)
    {
        if ([subveiw respondsToSelector:@selector(setEnabled:)]) {
            [subveiw setEnabled:YES];
        }
    }
}

- (void)windowDidResignKey:(id)sender
{
    for (id subveiw in _toolbarView.subviews)
    {
        if ([subveiw respondsToSelector:@selector(setEnabled:)]) {
            [subveiw setEnabled:NO];
        }
    }
}

#pragma mark -
#pragma mark NSTableView delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return _monitorEngine.arrangedItems.count;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    HWMItem *item = [_monitorEngine.arrangedItems objectAtIndex:row];

    NSUInteger height = [item isKindOfClass:[HWMGroup class]] ? 19 : 17;

    if (item.legend)
        height += 10;

    return height;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [_monitorEngine.arrangedItems objectAtIndex:row];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    HWMItem *item = [_monitorEngine.arrangedItems objectAtIndex:row];

    id view = [tableView makeViewWithIdentifier:item.identifier owner:self];

    return view;
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
    
    id item = [self.monitorEngine.arrangedItems objectAtIndex:[rowIndexes firstIndex]];
    
    if ([item isKindOfClass:[HWMSensor class]]) {
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorPopupItemDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorPopupItemDataType];

        return YES;
    }

    return NO;
}

-(void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    if (tableView == _tableView && (operation == NSDragOperationDelete || _currentItemDragOperation == NSDragOperationDelete))
    {
        NSPasteboard* pboard = [session draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPopupItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];
        id sourceItem = [self.monitorEngine.arrangedItems objectAtIndex:fromRow];

        [(HWMItem*)sourceItem setHidden:@YES];

        [_monitorEngine setNeedsUpdateLists];

        NSShowAnimationEffect(NSAnimationEffectPoof, screenPoint, NSZeroSize, nil, nil, nil);
    }
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
    id sourceItem = [self.monitorEngine.arrangedItems objectAtIndex:fromRow];

    _currentItemDragOperation = NSDragOperationNone;
    
    if ([sourceItem isKindOfClass:[HWMSensor class]] && toRow > 0) {
        
        _currentItemDragOperation = NSDragOperationMove;

        if (toRow < self.monitorEngine.arrangedItems.count) {

            if (toRow == fromRow || toRow == fromRow + 1) {
                _currentItemDragOperation = NSDragOperationNone;
            }
            else {
                id destinationItem = [self.monitorEngine.arrangedItems objectAtIndex:toRow];

                if ([destinationItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)sourceItem group] != [(HWMSensor*)destinationItem group]) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
                else {
                    destinationItem = [self.monitorEngine.arrangedItems objectAtIndex:toRow - 1];

                    if ([destinationItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)sourceItem group] != [(HWMSensor*)destinationItem group]) {
                        _currentItemDragOperation = NSDragOperationNone;
                    }
                }
            }
        }
        else {
            id destinationItem = [self.monitorEngine.arrangedItems objectAtIndex:toRow - 1];

            if ([destinationItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)sourceItem group] != [(HWMSensor*)destinationItem group]) {
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

    HWMSensor *sourceItem = [self.monitorEngine.arrangedItems objectAtIndex:fromRow];

    toRow = toRow > fromRow ? toRow - 1 : toRow;

    [_tableView moveRowAtIndex:fromRow toIndex:toRow];

    NSMutableArray *sensors = [[sourceItem.group.sensors sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"order" ascending:YES]]] mutableCopy];

    NSUInteger offset = [self.monitorEngine.arrangedItems indexOfObject:[sensors objectAtIndex:0]];

    [sensors removeObject:sourceItem];
    [sensors insertObject:sourceItem atIndex:offset <= toRow ? toRow - offset : 0];

    [sensors enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [obj setOrder:[NSNumber numberWithUnsignedInteger:idx]];
    }];

    //[self.monitorEngine setNeedsUpdateLists];

    return YES;
}

@end
