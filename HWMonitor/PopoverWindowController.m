//
//  PopoverWindowController.m
//  HWMonitor
//
//  Created by Kozlek on 23/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopoverWindowController.h"
#import "EXTKeyPathCoding.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "NSWindow+BackgroundBlur.h"
#import "Localizer.h"
#import "PopoverController.h"

@interface PopoverWindowController ()

@property (readonly) HWMEngine *monitorEngine;

@property (assign) IBOutlet NSView *toolbarView;
@property (nonatomic, strong) SensorsViewController *sensorsViewController;

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
    [super showWindow:sender];

    [self.window setHeavyBackgroundBlur];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    _sensorsViewController = [SensorsViewController new];

    [self.sensorsViewController setDelegate:self];

    [self.sensorsViewController.view setFrame:[self.window.contentView bounds]];

    //    [NSLayoutConstraint constraintWithItem:self.sensorsViewController.view
    //                                 attribute:NSLayoutAttributeLeft
    //                                 relatedBy:NSLayoutRelationEqual
    //                                    toItem:self.window.contentView
    //                                 attribute:NSLayoutAttributeLeft
    //                                multiplier:1.0
    //                                  constant:0];
    //    [NSLayoutConstraint constraintWithItem:self.sensorsViewController.view
    //                                 attribute:NSLayoutAttributeRight
    //                                 relatedBy:NSLayoutRelationEqual
    //                                    toItem:self.window.contentView
    //                                 attribute:NSLayoutAttributeRight
    //                                multiplier:1.0
    //                                  constant:0];
    //    [NSLayoutConstraint constraintWithItem:self.sensorsViewController.view
    //                                 attribute:NSLayoutAttributeTop
    //                                 relatedBy:NSLayoutRelationEqual
    //                                    toItem:self.window.contentView
    //                                 attribute:NSLayoutAttributeTop
    //                                multiplier:1.0
    //                                  constant:0];
    //    [NSLayoutConstraint constraintWithItem:self.sensorsViewController.view
    //                                 attribute:NSLayoutAttributeBottom
    //                                 relatedBy:NSLayoutRelationEqual
    //                                    toItem:self.window.contentView
    //                                 attribute:NSLayoutAttributeBottom
    //                                multiplier:1.0
    //                                  constant:0];

    [self.sensorsViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable | NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin];
    
    [self.window.contentView addSubview:self.sensorsViewController.view];

    [Localizer localizeView:self.window];

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
        [self close];
        [self.popoverController open:self];
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

    [self.window setFrame:NSMakeRect(self.window.frame.origin.x, self.window.frame.origin.y, self.window.frame.size.width, contentHeight) display:YES animate:YES];
}

#pragma mark - SensorsViewControllerDelegate

-(void)sensorsViewControllerDidReloadData:(SensorsViewController *)controller
{
    [self sizeToFitContent];
}

@end
