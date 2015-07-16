//
//  PopoverWindowController.m
//  HWMonitor
//
//  Created by Kozlek on 23/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopoverWindowController.h"
#import "PopoverWindow.h"
#import "EXTKeyPathCoding.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "NSWindow+BackgroundBlur.h"
#import "Localizer.h"
#import "PopoverController.h"
#import "SensorsViewController.h"
#import "NSWindow+ULIZoomEffect.h"

@interface PopoverWindowController () <SensorsViewControllerDelegate, PopoverWindowDelegate>

@property (readonly) HWMEngine *monitorEngine;

@property (assign) IBOutlet NSView *toolbarView;
@property (nonatomic, strong) SensorsViewController *sensorsViewController;

-(IBAction)closeWindow:(id)sender;
-(IBAction)attachToMenubar:(id)sender;
-(IBAction)showPreferencesWindow:(id)sender;
-(IBAction)showGraphsWindow:(id)sender;

@end

@implementation PopoverWindowController

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
}

-(instancetype)init
{
    self = [self initWithWindowNibName:NSStringFromClass([PopoverWindowController class])];

    if (self) {
        // Initialization code here.
    }

    return self;
}

-(void)showWindow:(id)sender
{
    NSRect statusItemFrame = self.popoverController.statusItemView.window.frame;

    [self.window makeKeyAndOrderFrontWithZoomEffectFromRect:statusItemFrame];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(NSEC_PER_SEC / 3)), dispatch_get_main_queue(), ^{
        [self.window setHeavyBackgroundBlur];
    });
}

-(void)closeWindow:(id)sender
{
    NSRect statusItemFrame = self.popoverController.statusItemView.window.frame;

    [self.window orderOutWithZoomEffectToRect:statusItemFrame];

    //    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(NSEC_PER_SEC / 3)), dispatch_get_main_queue(), ^{
    ////        [self close];
    //    });
}

- (void)windowDidLoad
{
    [super windowDidLoad];

    [Localizer localizeView:self.window.contentView];

    _sensorsViewController = [SensorsViewController new];
    [_sensorsViewController setDelegate:self];
    [_sensorsViewController.view setTranslatesAutoresizingMaskIntoConstraints:YES];
    [_sensorsViewController.view setAutoresizingMask:NSViewHeightSizable];
    [_sensorsViewController.view setFrame:[self.window.contentView bounds]];

    [self.contentViewController addChildViewController:_sensorsViewController];
    [self.window.contentView addSubview:_sensorsViewController.view];

    [self sizeToFitContent];
}

-(IBAction)showPreferencesWindow:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self.appController showWindow:self];
    }];
}

-(IBAction)attachToMenubar:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self closeWindow:sender];
    }];
}

-(IBAction)showGraphsWindow:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self.graphsController showWindow:self];
    }];
}

-(IBAction)showAboutWindow:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self.aboutController showWindow:self];
    }];
}

-(void)sizeToFitContent
{
    CGFloat contentHeight = _sensorsViewController.contentHeight + self.toolbarView.bounds.size.height + 5;
    CGFloat screenHeight = self.window.screen.visibleFrame.size.height - 20;

    if (contentHeight > screenHeight) {
        contentHeight = screenHeight;
        [_sensorsViewController.scrollView setHasVerticalScroller:YES];
    }
    else {
        [_sensorsViewController.scrollView setHasVerticalScroller:NO];
    }

    if (self.window.isVisible) {

        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {

            [self.window.animator setFrame:NSMakeRect(self.window.frame.origin.x, self.window.frame.origin.y, self.window.frame.size.width, contentHeight) display:YES];

        } completionHandler:^{
            [self.window setContentSize:NSMakeSize(self.window.frame.size.width, contentHeight)];
        }];
    }
    else {
        [self.window setContentSize:NSMakeSize(self.window.frame.size.width, contentHeight)];
    }
}

#pragma mark - SensorsViewControllerDelegate

-(void)sensorsViewControllerDidReloadData:(SensorsViewController *)controller
{
    [self sizeToFitContent];
}

#pragma mark - PopoverWindowDelegate

-(void)popoverWindowDidDoubleClick:(PopoverWindow *)window
{
    [self attachToMenubar:window];
}

@end
