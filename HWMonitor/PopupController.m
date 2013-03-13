//
//  PopupController.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  Based on code by Vadim Shpanovski <https://github.com/shpakovski/Popup>
//  Popup is licensed under the BSD license.
//  Copyright (c) 2013 Vadim Shpanovski, Natan Zalkin. All rights reserved.
//

#import "PopupController.h"

#import "HWMonitorDefinitions.h"
#import "HWMonitorGroup.h"

#import "GroupCell.h"
#import "SensorCell.h"
#import "BatteryCell.h"
#import "PopupView.h"

#define OPEN_DURATION .01
#define CLOSE_DURATION .15
#define MENU_ANIMATION_DURATION .1

@implementation PopupController

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

-(void)showWindow:(id)sender
{
    [self openPanel];
}

-(void)setColorTheme:(ColorTheme *)colorTheme
{
    _colorTheme = colorTheme;
    
    [_popupView setColorTheme:colorTheme];
    [_tableView reloadData];
}

- (id)init
{
    self = [super init];
    
    if (self != nil)
    {
        // Install status item into the menu bar
        _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        
        _statusItemView = [[StatusItemView alloc] initWithFrame:NSMakeRect(0, 0, 20, 20) statusItem:_statusItem];
        
        _statusItemView.image = [NSImage imageNamed:@"thermometer"];
        _statusItemView.alternateImage = [NSImage imageNamed:@"thermometer_template"];
        
        [_statusItem setHighlightMode:YES];
        
        [_statusItemView setAction:@selector(togglePanel:)];
        [_statusItemView setTarget:self];
    }
    
    return self;
}

- (void)dealloc
{
    [[NSStatusBar systemStatusBar] removeStatusItem:_statusItem];
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
    
    if ([view isKindOfClass:[NSWindow class]]) {
        [self localizeObject:[view contentView]];
    }
    else if ([view isKindOfClass:[NSView class]] ) {
        for(NSView *subView in [view subviews])
            [self localizeObject:subView];
    }
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    // Make a fully skinned panel
    NSPanel *panel = (id)[self window];
    [panel setAcceptsMouseMovedEvents:YES];
    [panel setLevel:NSPopUpMenuWindowLevel];
    [panel setOpaque:NO];
    [panel setBackgroundColor:[NSColor clearColor]];
    
    [self localizeObject:_prefsWindow];
    [self localizeObject:_graphsWindow];
    
    [_scrollView setFrameOrigin:NSMakePoint(_scrollView.frame.origin.x, - ARROW_HEIGHT)];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [self closePanel];
}

- (void)windowDidResignKey:(NSNotification *)notification;
{
    [self closePanel];
}

-(void)windowDidResignMain:(NSNotification *)notification
{
    [self closePanel];
}

- (void)cancelOperation:(id)sender
{
    [self closePanel];
}

- (void)windowDidResize:(NSNotification *)notification
{
    NSWindow *panel = [self window];
    NSRect statusRect = [self statusRectForWindow:panel];
    NSRect panelRect = [panel frame];
    
    CGFloat statusX = roundf(NSMidX(statusRect));
    CGFloat panelX = statusX - NSMinX(panelRect);
    
    self.popupView.arrowPosition = panelX;
}

- (NSRect)statusRectForWindow:(NSWindow *)window
{
    NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];
    NSRect statusRect = NSZeroRect;
    
    if (_statusItemView)
    {
        statusRect = _statusItemView.screenRect;
        statusRect.origin.y = NSMinY(statusRect) - NSHeight(statusRect);
    }
    else
    {
        statusRect.size = NSMakeSize(24, [[NSStatusBar systemStatusBar] thickness]);
        statusRect.origin.x = roundf((NSWidth(screenRect) - NSWidth(statusRect)) / 2.0);
        statusRect.origin.y = NSHeight(screenRect) - NSHeight(statusRect) * 2.0;
    }
    
    return statusRect;
}

-(void)togglePanel:(id)sender
{
    if (self.window)
    {
        if (self.window.isVisible)
        {
            [self closePanel];
            self.statusItemView.isHighlighted = NO;
        }
        else
        {
            [self openPanel];
            self.statusItemView.isHighlighted = YES;
        }
    }
}

- (void)openPanel
{
    if (self.window.isVisible)
        return;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupPanelShouldOpen:)]) {
        [self.delegate popupPanelShouldOpen:self];
    }
    
    NSWindow *panel = [self window];
    
    NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];
    NSRect statusRect = [self statusRectForWindow:panel];
    
    NSRect panelRect = [panel frame];
    panelRect.origin.x = roundf(NSMidX(statusRect) - NSWidth(panelRect) / 2);
    panelRect.origin.y = NSMaxY(statusRect) - NSHeight(panelRect) - ARROW_OFFSET;
    
    if (NSMaxX(panelRect) > (NSMaxX(screenRect) - ARROW_HEIGHT))
        panelRect.origin.x -= NSMaxX(panelRect) - (NSMaxX(screenRect) - ARROW_HEIGHT);
    
    [NSApp activateIgnoringOtherApps:NO];
    [panel setAlphaValue:0];
    //[panel setFrame:statusRect display:YES];
    [panel setFrame:panelRect display:YES];
    
    [panel makeKeyAndOrderFront:panel];
    //[panel orderFront:panel];
    
    NSTimeInterval openDuration = OPEN_DURATION;
    
    NSEvent *currentEvent = [NSApp currentEvent];
    if ([currentEvent type] == NSLeftMouseDown)
    {
        NSUInteger clearFlags = ([currentEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask);
        BOOL shiftPressed = (clearFlags == NSShiftKeyMask);
        BOOL shiftOptionPressed = (clearFlags == (NSShiftKeyMask | NSAlternateKeyMask));
        if (shiftPressed || shiftOptionPressed)
        {
            openDuration *= 10;
            
            if (shiftOptionPressed)
                NSLog(@"Icon is at %@\n\tMenu is on screen %@\n\tWill be animated to %@",
                      NSStringFromRect(statusRect), NSStringFromRect(screenRect), NSStringFromRect(panelRect));
        }
    }
    
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:openDuration];
    //[[panel animator] setFrame:panelRect display:YES];
    [[panel animator] setAlphaValue:1];
    [NSAnimationContext endGrouping];
    
    [self windowDidResize:nil];
    
    self.statusItemView.isHighlighted = YES;
}

- (void)closePanel
{
    if (!self.window.isVisible)
        return;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupPanelShouldClose:)]) {
        [self.delegate popupPanelShouldClose:self];
    }
    
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:CLOSE_DURATION];
    [[[self window] animator] setAlphaValue:0];
    [NSAnimationContext endGrouping];
    
    dispatch_after(dispatch_walltime(NULL, NSEC_PER_SEC * CLOSE_DURATION * 2), dispatch_get_main_queue(), ^{
        [self.window orderOut:nil];
    });
    
    self.statusItemView.isHighlighted = NO;
}

- (IBAction)closeApplication:(id)sender
{
    [NSApp terminate:nil];
}

- (IBAction)openPreferences:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [_prefsWindow makeKeyAndOrderFront:nil];
}

- (IBAction)showGraphs:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [_graphsWindow makeKeyAndOrderFront:nil];
}

- (void) setupWithGroups:(NSArray*)groups
{
    _items = [[NSMutableArray alloc] init];
    
    // Add special toolbar item
    [_items addObject:@"Toolbar"];
    
    if ([groups count] > 0) {
        for (HWMonitorGroup *group in groups) {
            if ([group checkVisibility]) {
                [_items addObject:group];
                
                for (HWMonitorItem *item in [group items]) {
                    if ([item isVisible]) {
                        [_items addObject:item];
                    }
                }
            }
        }
    }
    else {
        [_items addObject:@"Dummy"];
    }
    
    [self reloadData];
}

- (void) reloadData
{
    [_tableView reloadData];
    
    // Make window height small
    NSRect panelRect = [[self window] frame];
    panelRect.size.height = ARROW_HEIGHT + kHWMonitorToolbarHeight;
    [[self window] setFrame:panelRect display:NO];
    
    // Resize panel height to fit all table view content
    panelRect.size.height = [_tableView frame].size.height + ARROW_HEIGHT + CORNER_RADIUS;
    [[self window] setFrame:panelRect display:NO];
}

- (void)updateValues
{
    for (NSUInteger index = 0; index < [_items count]; index++) {
        
        id item = [_items objectAtIndex:index];
        
        if ([item isKindOfClass:[HWMonitorItem class]]) {
            SensorCell *cell = [_tableView viewAtColumn:0 row:index makeIfNecessary:NO];
            HWMonitorSensor *sensor = [item sensor];
            
            if ([sensor valueHasBeenChanged]) {
                
                NSColor *valueColor;
                
                switch ([sensor level]) {
                        /*case kHWSensorLevelDisabled:
                         break;
                         
                         case kHWSensorLevelNormal:
                         break;*/
                        
                    case kHWSensorLevelModerate:
                        valueColor = [NSColor colorWithCalibratedRed:0.7f green:0.3f blue:0.03f alpha:1.0f];
                        break;
                        
                    case kHWSensorLevelExceeded:
                        [cell.textField setTextColor:[NSColor redColor]];
                    case kHWSensorLevelHigh:
                        valueColor = [NSColor redColor];
                        break;
                        
                    default:
                        valueColor = _colorTheme.itemValueTitleColor;
                        break;
                }
                
                [cell.valueField setStringValue:[sensor formattedValue]];
                [cell.valueField setTextColor:valueColor];
            }
        }
    }
}

// NSTableView delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [_items count];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{    
    id item = [_items objectAtIndex:row];
    
    if ([item isKindOfClass:[HWMonitorGroup class]]) {
        return kHWMonitorGroupHeight;
    }
    else if ([item isKindOfClass:[HWMonitorItem class]]) {
        HWMonitorSensor *sensor = [item sensor];
        
        if (_showVolumeNames && [sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[ATAGenericDisk class]]) {
            return kHWMonitorSubtitledHeight;
        }
        else {
            return kHWMonitorSensorHeight;
        }
    }
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Toolbar"]) {
        return kHWMonitorToolbarHeight;
    }
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Battery"]) {
        return kHWMonitorBatteryHeight;
    }

    return  kHWMonitorSensorHeight;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return false;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id item = [_items objectAtIndex:row];
    
    if ([item isKindOfClass:[HWMonitorGroup class]]) {
        HWMonitorGroup *group = item;
        
        GroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];
        
        [groupCell setColorTheme:_colorTheme];
        [groupCell.textField setStringValue:[group title]];
        [groupCell.imageView setObjectValue:_colorTheme.useAlternateImages ? [[group icon] alternateImage] : [[group icon] image]];
        
        return groupCell;
    }
    else if ([item isKindOfClass:[HWMonitorItem class]]) {
        HWMonitorSensor *sensor = [item sensor];
        
        SensorCell *sensorCell = nil;
        
        if (([sensor group] & kHWSensorGroupTemperature) || ([sensor group] & kSMARTGroupTemperature)) {
            sensorCell = [tableView makeViewWithIdentifier:@"Temperature" owner:self];
        }
        else if (([sensor group] & kHWSensorGroupPWM) || ([sensor group] & kSMARTGroupRemainingLife)) {
            sensorCell = [tableView makeViewWithIdentifier:@"Percentage" owner:self];
        }
        else {
            sensorCell = [tableView makeViewWithIdentifier:@"Sensor" owner:self];
        }
        
        if (_showVolumeNames && [sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[ATAGenericDisk class]]) {
            [sensorCell.subtitleField setTextColor:_colorTheme.itemSubTitleColor];
            [sensorCell.subtitleField setStringValue:[[sensor genericDevice] volumesNames]];
            [sensorCell.subtitleField setHidden:NO];
        }
        else {
            [sensorCell.subtitleField setHidden:YES];
        }
        
        [sensorCell.textField setTextColor:_colorTheme.itemTitleColor];
        [sensorCell.textField setStringValue:[sensor title]];
        [sensorCell.valueField setTextColor:_colorTheme.itemValueTitleColor];
        [sensorCell.valueField setStringValue:[sensor formattedValue]];
        
        return sensorCell;
    }
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Toolbar"]) {
        NSTableCellView *buttonsCell = [tableView makeViewWithIdentifier:item owner:self];
        
        [buttonsCell.textField setTextColor:_colorTheme.barTitleColor];
        
        return buttonsCell;
    }
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Dummy"]) {
        NSTableCellView *dummyCell = [tableView makeViewWithIdentifier:item owner:self];
        
        [dummyCell.textField setTextColor:_colorTheme.itemTitleColor];
        
        return dummyCell;
    }
    
    return nil;
}

@end
