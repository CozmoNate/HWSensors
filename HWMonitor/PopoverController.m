//
//  PopoverViewController.m
//  HWMonitor
//
//  Created by Kozlek on 19/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopoverController.h"
#import "PopoverWindowController.h"

#import "StatusItemView.h"
#import "NSImage+HighResolutionLoading.h"

#import "EXTKeyPathCoding.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMIcon.h"

#import "PopoverFrameView.h"

#import "Localizer.h"
#import "PopupSensorCell.h"

@interface PopoverController ()
{
    PopoverWindowController * _popoverWindowController;
}

@property (readonly) HWMEngine *monitorEngine;

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (nonatomic, strong) NSPopover *popover;
@property (nonatomic, strong) SensorsViewController *sensorsViewController;

-(IBAction)showPreferencesWindow:(id)sender;
-(IBAction)showGraphsWindow:(id)sender;
-(IBAction)showAboutWindow:(id)sender;

@end

@implementation PopoverController

#pragma mark - Properties

@synthesize popover = _popover;

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
}

-(NSPopover*)popover
{
    if (!_popover) {
        _popover = [NSPopover new];

        [_popover setDelegate:self];

//        [_popover setAnimates:NO];
        [_popover setBehavior:NSPopoverBehaviorTransient];

        [self colorThemeChanged];

        [_popover setContentViewController:self];

        [self sizePopoverToFitContent];
//        [_popover setAnimates:YES];
    }

    return _popover;
}

-(void)setPopover:(NSPopover *)popover
{
    if (_popover) {
        [_popover close];
    }

    _popover = popover;
}

-(BOOL)isShown
{
    return (self.popover && self.popover.isShown) || (_popoverWindowController && _popoverWindowController.window.isVisible);
}

-(BOOL)isDetached
{
    return _popoverWindowController.window.isVisible;
}

#pragma mark - Overridden

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:NSStringFromClass([PopoverController class]) bundle:[NSBundle mainBundle]];

    if (self) {

        _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];

//        if ([_statusItem respondsToSelector:@selector(button)]) {
//            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(redrawStatusItem) name:HWMEngineSensorValuesHasBeenUpdatedNotification object:self.monitorEngine];
//
//            [[_statusItem button] setAction:@selector(toggle:)];
//            [[_statusItem button] setTarget:self];
//        }
//        else {
            _statusItemView = [[StatusItemView alloc] initWithFrame:NSMakeRect(0, 0, 22, 22) statusItem:_statusItem];

            [_statusItemView setImage:[NSImage loadImageNamed:@"scale" ofType:@"png"]];
            [_statusItemView setAlternateImage:[NSImage loadImageNamed:@"scale-white" ofType:@"png"]];

            [_statusItemView setAction:@selector(toggle:)];
            [_statusItemView setTarget:self];
//        }

        PopoverFrameView * toolbar = (PopoverFrameView *)self.view;
        [toolbar setToolbarTitle:@"HWMonitor"];

        _sensorsViewController = [SensorsViewController new];
        _sensorsViewController.delegate = self;

        [_sensorsViewController.view setFrame:NSMakeRect(0, 0, self.view.bounds.size.width, self.view.bounds.size.height - toolbar.toolbarHeight)];
        [_sensorsViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

        [self.view addSubview:_sensorsViewController.view];

        [PopupSensorCell setGlobalPopoverDelegate:self];

        [[NSOperationQueue mainQueue] addOperationWithBlock:^{

            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(close:) name:NSApplicationDidResignActiveNotification object:[NSApplication sharedApplication]];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(close:) name:NSApplicationDidHideNotification object:[NSApplication sharedApplication]];

            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(close:) name:NSWindowDidResignKeyNotification object:self];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(close:) name:NSWindowDidResignMainNotification object:self];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(close:) name:NSWindowDidBecomeKeyNotification object:nil];

            [self addObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme) options:0 context:nil];

            [Localizer localizeView:self.view];
        }];
    }
    return self;
}

-(void)mouseUp:(NSEvent *)theEvent
{
    if (theEvent.clickCount == 2) {
        [self detach:self];
    }
}

-(void)dealloc
{
    [self removeObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme)];
    [[NSStatusBar systemStatusBar] removeStatusItem:_statusItem];
}

#pragma mark - Actions

-(IBAction)open:(id)sender
{
//    NSLog(@"open:%@", sender);

    [NSApp activateIgnoringOtherApps:YES];

    if (_popoverWindowController && _popoverWindowController.window.isVisible) {
        if (!_popoverWindowController.window.isKeyWindow) {
            [_popoverWindowController.window makeKeyAndOrderFront:self];
            return;
        }
        else {
            return;
        }
    }
    
//    [_popover setAnimates:NO];

    [_popover showRelativeToRect:_statusItemView.frame ofView:_statusItemView preferredEdge:NSMinYEdge];
    [self colorThemeChanged];
    [self sizePopoverToFitContent];

//    [_popover setAnimates:YES];

}

-(IBAction)close:(id)sender
{
//    NSLog(@"close:%@", sender);

    if (_popoverWindowController.window.isVisible) {

//        if (sender != self && ![sender isKindOfClass:[NSNotification class]]) {
//            [_popoverWindowController close];
//            _popoverWindowController = nil;
//        }

        return;
    }

    if ([sender isKindOfClass:[NSNotification class]]) {
        NSNotification * note = (NSNotification*)sender;

        if (note.name == NSWindowDidBecomeKeyNotification && note.object == self.view.window) {
            return;
        }
    }

    [self closePopover:sender];
}

-(IBAction)toggle:(id)sender
{
    if ((_popover && _popover.isShown) /*|| (_popoverWindowController && _popoverWindowController.window.isVisible)*/) {
        [self close:self];
    }
    else {
        [self open:self];
    }
}

-(IBAction)detach:(id)sender
{
    [self close:self];

    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        if (!_popoverWindowController) {
            _popoverWindowController = [PopoverWindowController new];
            [_popoverWindowController setPopoverController:self];
            [_popoverWindowController setAppController:_appController];
            [_popoverWindowController setGraphsController:_graphsController];
            [_popoverWindowController setAboutController:_aboutController];
        }

        [_popoverWindowController showWindow:sender];
    }];
}

-(IBAction)closePopover:(id)sender
{
    if ( _popover && _popover.isShown) {
        //[_popover setAnimates:NO];
        [_popover performClose:sender];
        //[_popover setAnimates:YES];
    }
}

-(IBAction)showPreferencesWindow:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self.appController showWindow:sender];
    }];

   [self closePopover:self];
}

-(IBAction)showGraphsWindow:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self.graphsController showWindow:sender];
    }];

    [self closePopover:self];
}

- (void)showAboutWindow:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [_aboutController showWindow:sender];
    }];

    [self closePopover:self];
}

#pragma mark - Methods

-(void)redrawStatusItem
{
    //NSArray * favorites = [self.monitorEngine.favorites copy];

    //if (!favorites.count) {
    //    [_statusItem.button setImage:[NSImage imageNamed:@"scale"]];
    //}
    //else {

    //}
}

//-(void)makePopover
//{
//    if (!_popover) {
//        _popover = [NSPopover new];
//
//        [_popover setDelegate:self];
//
//        [_popover setAnimates:NO];
//        [_popover setBehavior:NSPopoverBehaviorTransient];
//
//        [self colorThemeChanged];
//
//        [_popover setContentViewController:self];
//    }
//
//    [self sizePopoverToFitContent];
//}

-(void)colorThemeChanged
{
    if ([NSAppearance class]) {
        [self.popover.contentViewController.view.window setAppearance:[NSAppearance appearanceNamed:self.monitorEngine.configuration.colorTheme.useBrightIcons.boolValue ? NSAppearanceNameVibrantDark : NSAppearanceNameAqua]];
//        [self.popover setAppearance:self.monitorEngine.configuration.colorTheme.useBrightIcons.boolValue ? NSAppearanceNameVibrantDark : NSAppearanceNameAqua];
    }

    [self.popover setAppearance:self.monitorEngine.configuration.colorTheme.useBrightIcons.boolValue ? NSPopoverAppearanceHUD : NSPopoverAppearanceMinimal];
}

-(void)sizePopoverToFitContent
{
    if (_popover && _popover.isShown) {
        CGFloat contentHeight = _sensorsViewController.contentHeight + [(PopoverFrameView*)self.view toolbarHeight] + 5;
        CGFloat screenHeight = NSScreen.mainScreen.visibleFrame.size.height - 20;

        if (contentHeight > screenHeight) {
            contentHeight = screenHeight;
            [_sensorsViewController.scrollView setHasVerticalScroller:YES];
        }
        else {
            [_sensorsViewController.scrollView setHasVerticalScroller:NO];
        }

        [_popover setContentSize:NSMakeSize(self.view.bounds.size.width, contentHeight)];
    }
}

#pragma mark - Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.colorTheme)]) {
        self.popover = nil;
        [self colorThemeChanged];
    }
}

#pragma mark - NSPopoverDelegate

-(void)popoverWillShow:(NSNotification *)notification
{
    if (notification.object != _popover) {
        [self.monitorEngine updateSmcAndDeviceSensors];
        [self.monitorEngine updateAtaSmartSensors];
        return;
    }

    if (self.popoverDelegate && [self.popoverDelegate respondsToSelector:@selector(popoverWillShow:)]) {
        [self.popoverDelegate popoverWillShow:notification];
    }
}

-(void)popoverDidShow:(NSNotification *)notification
{
    if (notification.object != _popover) {
        return;
    }

    if (self.popoverDelegate && [self.popoverDelegate respondsToSelector:@selector(popoverDidShow:)]) {
        [self.popoverDelegate popoverDidShow:notification];
    }
}

-(void)popoverWillClose:(NSNotification *)notification
{
    if (notification.object != _popover) {
        return;
    }

    if (self.popoverDelegate && [self.popoverDelegate respondsToSelector:@selector(popoverWillClose:)]) {
        [self.popoverDelegate popoverWillClose:notification];
    }
}

-(void)popoverDidClose:(NSNotification *)notification
{
    if (notification.object != _popover) {
        if (![NSApp isActive] || self.isDetached /*||
            self.aboutController.window.isVisible || self.appController.window.isVisible || self.graphsController.window.isVisible*/) {
            [self closePopover:self];
        }
        return;
    }
    
    if (self.popoverDelegate && [self.popoverDelegate respondsToSelector:@selector(popoverDidClose:)]) {
        [self.popoverDelegate popoverDidClose:notification];
    }
}

-(BOOL)popoverShouldDetach:(NSPopover *)popover
{
    return YES;
}

-(NSWindow *)detachableWindowForPopover:(NSPopover *)popover
{

    if (!_popoverWindowController) {
        _popoverWindowController = [PopoverWindowController new];
    
        [_popoverWindowController setPopoverController:self];
        [_popoverWindowController setAppController:_appController];
        [_popoverWindowController setGraphsController:_graphsController];
        [_popoverWindowController setAboutController:_aboutController];
    }

    return _popoverWindowController.window;
}

#pragma mark - SensorsViewControllerDelegate

-(void)sensorsViewControllerDidReloadData:(SensorsViewController *)controller
{
    [self sizePopoverToFitContent];
}

@end
