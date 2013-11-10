//
//  AboutController.m
//  HWMonitor
//
//  Created by Kozlek on 30/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "AboutController.h"

#import "FadingButton.h"
#import "Localizer.h"

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

    NSMutableAttributedString* creditsContent = [[NSMutableAttributedString alloc] initWithPath:[[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtfd"] documentAttributes:nil];

    [[creditsContent mutableString] replaceOccurrencesOfString:@"%version_placeholder" withString:[NSString stringWithFormat:GetLocalizedString(@"Version %@"), [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"]] options:NSCaseInsensitiveSearch range:NSMakeRange(0, creditsContent.mutableString.length)];

    [[self.textView textStorage] setAttributedString:creditsContent];
    
    [self.copyrightField setStringValue:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSHumanReadableCopyright"]];

    [[self creditsScrollView] startScrollWithStartTime:3.0f position:0.0f];
}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];

    [self.window setLevel:NSFloatingWindowLevel];
    
    [super showWindow:sender];

    [[self creditsScrollView] startScrollWithStartTime:3.0f position:0.0f];
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

@end
