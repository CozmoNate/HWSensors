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
#import "WindowFilter.h"

#define OPEN_DURATION .01
#define CLOSE_DURATION .15
#define MENU_ANIMATION_DURATION .1

@implementation PopupController

-(void)setColorTheme:(ColorTheme *)colorTheme
{
    _colorTheme = colorTheme;
    
    [_popupView setColorTheme:colorTheme];
    [_tableView reloadData];
}

- (void)setupPanel
{
    // Make a fully skinned panel
    NSPanel *panel = (id)self.window;
    
    [panel setAcceptsMouseMovedEvents:YES];
    [panel setLevel:NSPopUpMenuWindowLevel];
    [panel setOpaque:NO];
    [panel setBackgroundColor:[NSColor clearColor]];
}

- (void)awakeFromNib
{
    [self resizeToContentAnimated:NO orderFront:NO];
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
        
        
        [self performSelector:@selector(setupPanel) withObject:nil afterDelay:0.0];
    }
    
    return self;
}

-(void)showWindow:(id)sender
{
    [self openPanel];
}

-(void)close
{
    [self closePanel];
}

- (void)dealloc
{
    [[NSStatusBar systemStatusBar] removeStatusItem:_statusItem];
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

//- (void)windowDidResize:(NSNotification *)notification
//{
//    NSWindow *panel = [self window];
//    NSRect statusRect = [self statusRectForWindow:panel];
//    NSRect panelRect = [panel frame];
//    
//    CGFloat statusX = roundf(NSMidX(statusRect));
//    CGFloat panelX = statusX - NSMinX(panelRect);
//    
//    self.popupView.arrowPosition = panelX;
//}

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
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupWillOpen:)]) {
        [self.delegate popupWillOpen:self];
    }
    
    // Update values
    for (id item in _items) {
        if ([item isKindOfClass:[HWMonitorItem class]]) {
            [self updateValueForItem:item];
        }
    }
    
    [self.window setLevel:NSPopUpMenuWindowLevel];
    [self.window setAlphaValue:1.0];
    [self resizeToContentAnimated:NO orderFront:YES];
    
    if (!_windowFilter) {
        _windowFilter = [[WindowFilter alloc] initWithWindow:self.window name:@"CIGaussianBlur" andOptions:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.5] forKey:@"inputRadius"]];
    }
    else {
        [_windowFilter setFilterOptions:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.5] forKey:@"inputRadius"]];
    }
    
    self.statusItemView.isHighlighted = YES;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupDidOpen:)]) {
        [self.delegate popupDidOpen:self];
    }
}

- (void)closePanel
{
    if (!self.window.isVisible)
        return;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupWillClose:)]) {
        [self.delegate popupWillClose:self];
    }
    
    if (_windowFilter) {
        [_windowFilter setFilterOptions:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:0.0] forKey:@"inputRadius"]];
    }
    
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:CLOSE_DURATION];
    [self.window.animator setAlphaValue:0];
    [NSAnimationContext endGrouping];
    
    dispatch_after(dispatch_walltime(NULL, NSEC_PER_SEC * CLOSE_DURATION * 2), dispatch_get_main_queue(), ^{
        [self.window orderOut:nil];
    });
    
    self.statusItemView.isHighlighted = NO;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupDidClose:)]) {
        [self.delegate popupDidClose:self];
    }
}

- (IBAction)closeApplication:(id)sender
{
    [NSApp terminate:nil];
}

- (IBAction)showAboutWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp orderFrontStandardAboutPanel:nil];
}

- (IBAction)openPreferences:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [_appController showWindow:self];
}

- (IBAction)showGraphs:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [_graphsController showWindow:self];
}

- (void) setupWithGroups:(NSArray*)groups
{
    _items = [[NSMutableArray alloc] init];
    
    // Add special toolbar item
    //[_items addObject:@"Toolbar"];
    
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

- (void)resizeToContentAnimated:(BOOL)animated orderFront:(BOOL)orderFront
{
    NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];
    NSRect statusRect = [self statusRectForWindow:self.window];
    NSRect originalRect = self.window.frame;
    NSRect panelRect = self.window.frame;
    
    // Make window small
    [self.window setFrame:NSMakeRect(0, 0, 8, 8) display:NO];
    
    // Resize panel height to fit table view content and toolbar
    panelRect.size.height = ARROW_HEIGHT + kHWMonitorToolbarHeight + _tableView.frame.size.height + CORNER_RADIUS * 2;
    
    // Check panel is inside the screen bounds
    if ([[NSScreen mainScreen] visibleFrame].size.height < panelRect.size.height) {
        panelRect.size.height = [[NSScreen mainScreen] visibleFrame].size.height - ARROW_OFFSET * 2;
    }
    
    panelRect.origin.x = roundf(NSMidX(statusRect) - NSWidth(panelRect) / 2);
    panelRect.origin.y = NSMaxY(statusRect) - NSHeight(panelRect) - ARROW_OFFSET;
    
    if (NSMaxX(panelRect) > (NSMaxX(screenRect) - ARROW_HEIGHT))
        panelRect.origin.x -= NSMaxX(panelRect) - (NSMaxX(screenRect) - ARROW_HEIGHT);
    
    // Back to previous frame
    [self.window setFrame:originalRect display:NO];
    
    // Update arrow position
    [self.popupView setArrowPosition:roundf(NSMidX(statusRect)) - NSMinX(panelRect)];
    
    // Order front if needed
    if (animated) {
        if (orderFront) {
            [self.window makeKeyAndOrderFront:self];
        }
        
        [self.window setFrame:panelRect display:YES animate:YES];
    }
    else {
        [self.window setFrame:panelRect display:YES animate:NO];
        
        if (orderFront) {
            [self.window makeKeyAndOrderFront:self];
        }
    }
}

- (void)reloadData
{
    [_tableView reloadData];
    [self resizeToContentAnimated:YES orderFront:NO];
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
            
            if ([item.sensor genericDevice] && [[item.sensor genericDevice] isKindOfClass:[BluetoothGenericDevice class]]) {
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

// NSTableView delegate

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
            ([sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[BluetoothGenericDevice class]] && [[sensor genericDevice] productName])) {
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
        else if ([sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[BluetoothGenericDevice class]]) {
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
