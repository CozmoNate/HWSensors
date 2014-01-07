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

#import <WebKit/WebKit.h>

@interface AboutController ()

@end

@implementation AboutController

-(id)init
{
    self = [super initWithWindowNibName:@"AboutController"];
    
    if (self) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            _creditsUrl = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"html"]];

            NSError *error;

            NSMutableString *html = [[[NSString alloc] initWithContentsOfURL:_creditsUrl encoding:NSUTF8StringEncoding error:&error] mutableCopy];

            [html replaceOccurrencesOfString:@"%version_placeholder" withString:[NSString stringWithFormat:@"Version %@", [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"]] options:NSCaseInsensitiveSearch range:NSMakeRange(0, html.length)];

            _creditsHtml = [html copy];
        }];
    }
    
    return self;
}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [super showWindow:sender];

    _webViewFinishedLoading = NO;
    [[_creditsWebView mainFrame] loadHTMLString:_creditsHtml baseURL:_creditsUrl];
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
    _webViewFinishedLoading = NO;
    
    [[_creditsWebView mainFrame] loadHTMLString:nil baseURL:nil];
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:frame
{
    if ([frame parentFrame] == nil) {
        _webViewFinishedLoading = YES;
    }
}

- (void)webView:sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:frame decisionListener:listener
{
    if (_webViewFinishedLoading) {
        [[NSWorkspace sharedWorkspace] openURL:[request URL]];
        [listener ignore];
    }
    else {
        [listener use];
    }
}

@end
