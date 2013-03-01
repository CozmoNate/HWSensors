//
//  PopupController.m
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

#import "PopupController.h"

#import "HWMonitorDefinitions.h"
#import "HWMonitorGroup.h"

#import "GroupCell.h"
#import "SensorCell.h"
#import "PopupView.h"

#define OPEN_DURATION .01
#define CLOSE_DURATION .15
#define MENU_ANIMATION_DURATION .1

@implementation PopupController

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

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
    [self setHasActivePanel:NO];
}

- (void)windowDidResignKey:(NSNotification *)notification;
{
    if ([[self window] isVisible])
    {
        [self setHasActivePanel:NO];
    }
}

- (void)cancelOperation:(id)sender
{
    [self setHasActivePanel:NO];
}

- (void)windowDidResize:(NSNotification *)notification
{
    NSWindow *panel = [self window];
    NSRect statusRect = [self statusRectForWindow:panel];
    NSRect panelRect = [panel frame];
    
    CGFloat statusX = roundf(NSMidX(statusRect));
    CGFloat panelX = statusX - NSMinX(panelRect);
    
    self.popupView.arrowPosition = panelX;
    
    /*NSRect searchRect = [self.searchField frame];
    searchRect.size.width = NSWidth([self.backgroundView bounds]) - SEARCH_INSET * 2;
    searchRect.origin.x = SEARCH_INSET;
    searchRect.origin.y = NSHeight([self.backgroundView bounds]) - ARROW_HEIGHT - SEARCH_INSET - NSHeight(searchRect);
    
    if (NSIsEmptyRect(searchRect))
    {
        [self.searchField setHidden:YES];
    }
    else
    {
        [self.searchField setFrame:searchRect];
        [self.searchField setHidden:NO];
    }
    
    NSRect textRect = [self.textField frame];
    textRect.size.width = NSWidth([self.backgroundView bounds]) - SEARCH_INSET * 2;
    textRect.origin.x = SEARCH_INSET;
    textRect.size.height = NSHeight([self.backgroundView bounds]) - ARROW_HEIGHT - SEARCH_INSET * 3 - NSHeight(searchRect);
    textRect.origin.y = SEARCH_INSET;
    
    if (NSIsEmptyRect(textRect))
    {
        [self.textField setHidden:YES];
    }
    else
    {
        [self.textField setFrame:textRect];
        [self.textField setHidden:NO];
    }*/
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
        statusRect.origin.x = roundf((NSWidth(screenRect) - NSWidth(statusRect)) / 2);
        statusRect.origin.y = NSHeight(screenRect) - NSHeight(statusRect) * 2;
    }
    
    return statusRect;
}

- (BOOL)hasActivePanel
{
    return _hasActivePanel;
}

- (void)setHasActivePanel:(BOOL)hasActivePanel
{
    if (_hasActivePanel != hasActivePanel)
    {
        _hasActivePanel = hasActivePanel;
        
        if (_hasActivePanel)
        {
            [self openPanel];
        }
        else
        {
            [self closePanel];
        }
        
        self.statusItemView.isHighlighted = hasActivePanel;
    }
}

- (void)openPanel
{
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
    
    //[panel performSelector:@selector(makeFirstResponder:) withObject:self.searchField afterDelay:openDuration];
}

- (void)closePanel
{
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:CLOSE_DURATION];
    [[[self window] animator] setAlphaValue:0];
    [NSAnimationContext endGrouping];
    
    dispatch_after(dispatch_walltime(NULL, NSEC_PER_SEC * CLOSE_DURATION * 2), dispatch_get_main_queue(), ^{
        [self.window orderOut:nil];
    });
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
    
    // Add special control buttons item
    [_items addObject:@"Buttons"];
    
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
    panelRect.size.height = ARROW_HEIGHT + 35;
    [[self window] setFrame:panelRect display:NO];
    
    // Resize panel height to fit all table view content
    panelRect.size.height = [_tableView frame].size.height + ARROW_HEIGHT + CORNER_RADIUS;
    [[self window] setFrame:panelRect display:NO];
}

- (void) updateValues
{
    for (NSUInteger index = 0; index < [_items count]; index++) {
        
        id item = [_items objectAtIndex:index];
        
        if ([item isKindOfClass:[HWMonitorItem class]]) {
            SensorCell *cell = [_tableView viewAtColumn:0 row:index makeIfNecessary:NO];
            HWMonitorSensor *sensor = [item sensor];
            
            if ([sensor valueHasBeenChanged]) {
                [cell.valueField setStringValue:[sensor formattedValue]];
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
        
        if (_showVolumeNames && [sensor disk]) {
            return kHWMonitorSubtitledHeight;
        }
        else {
            return kHWMonitorSensorHeight;
        }
    }
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Buttons"]) {
        return kHWMonitorButtonsHeight;
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
        
        if (([sensor group] & kHWSensorGroupTemperature) || ([sensor group] & kSMARTSensorGroupTemperature)) {
            sensorCell = [tableView makeViewWithIdentifier:@"Temperature" owner:self];
        }
        else {
            sensorCell = [tableView makeViewWithIdentifier:@"Sensor" owner:self];
        }
        
        if (_showVolumeNames && [sensor disk]) {
            [sensorCell.subtitleField setTextColor:_colorTheme.itemSubTitleColor];
            [sensorCell.subtitleField setStringValue:[[sensor disk] volumesNames]];
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
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Buttons"]) {
        NSTableCellView *buttonsCell = [tableView makeViewWithIdentifier:@"Buttons" owner:self];
        
        [buttonsCell.textField setTextColor:_colorTheme.barTitleColor];
        
        return buttonsCell;
    }
    else if ([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Dummy"]) {
        return [tableView makeViewWithIdentifier:@"Dummy" owner:self];
    }
    
    return nil;
}

@end
