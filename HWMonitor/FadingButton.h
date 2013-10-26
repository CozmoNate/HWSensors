//
//  FadingButton.h
//  HWMonitor
//
//  Created by kozlek on 28.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

@interface FadingButton : NSButton

@property (strong) NSString *actionURL;
@property (assign) CGFloat normalOpacity;
@property (assign) CGFloat hoverOpacity;
@property (assign) CGFloat downOpacity;

- (IBAction)fadeIn:(id)sender;
- (IBAction)fadeOut:(id)sender;

@end
