//
//  AboutController.m
//  HWMonitor
//
//  Created by Kozlek on 30/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "AboutController.h"

#import "FadingButton.h"

@interface AboutController ()

@end

@implementation AboutController

-(id)init
{
    self = [super initWithWindowNibName:@"AboutController"];
    
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    [self.versionField setStringValue:[NSString stringWithFormat:NSLocalizedString(@"Version %@", @"Localize the string App Version in AboutController"), [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"]]];
    
    [_creditsScrollView setCurrentPosition:0.0f];
    [_creditsScrollView setRestartAtTop:NO];
    [_creditsScrollView setStartTime:[NSDate timeIntervalSinceReferenceDate] + 1.0];
    
    NSMutableAttributedString* creditsContent = [[NSMutableAttributedString alloc] initWithPath:[[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"]
                                                                             documentAttributes:nil];
    
    [creditsContent insertAttributedString:[[NSAttributedString alloc] initWithString:@"\n\n\n\n\n\n\n\n\n\n\n\n" attributes:nil] atIndex:0];
    [creditsContent insertAttributedString:[[NSAttributedString alloc] initWithString:@"\n\n\n\n\n\n\n\n\n\n\n\n" attributes:nil] atIndex:creditsContent.length];
    
    
    [[[[self creditsScrollView] textView] textStorage] setAttributedString:creditsContent];
    
    [self.copyrightField setStringValue:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSHumanReadableCopyright"]];
    
    [[self creditsScrollView] startScroll];
}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    
    [super showWindow:sender];
}

- (void)showCredits:(id)sender
{
    [[NSWorkspace sharedWorkspace] openFile:[[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"]];
}

- (void)openLink:(id)sender
{
    if ([sender isKindOfClass:[FadingButton class]]) {
        FadingButton *button = sender;
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[button actionURL]]];
    }
}

- (void)windowWillClose:(NSNotification *)notification
{
    [_creditsScrollView stopScroll];
}

- (NSString*)localizeCreditsButton
{
    return NSLocalizedString(@"Credits...", @"Localize credits button in AboutController");
}

@end
