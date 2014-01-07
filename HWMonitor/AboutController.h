//
//  AboutController.h
//  HWMonitor
//
//  Created by Kozlek on 30/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WebView;

@interface AboutController : NSWindowController
{
    NSURL *_creditsUrl;
    NSString *_creditsHtml;
    BOOL _webViewFinishedLoading;
}

@property (assign) IBOutlet WebView *creditsWebView;

@end
