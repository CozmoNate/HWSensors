//
//  NSWindow+BackgroundGaussianBlur.h
//  HWMonitor
//
//  Created by Kozlek on 15.03.14.
//  Copyright (c) 2014 Guilherme Rambo, kozlek. All rights reserved.
//

// Original article: http://stackoverflow.com/questions/19575642/how-to-use-cifilter-on-nswindow-in-osx-10-9-mavericks

#import <Cocoa/Cocoa.h>

@interface NSWindow (BackgroundBlur)

@property (nonatomic, assign) NSInteger backgroundBlurRadius;

- (void)setBackgroundBlurRadius:(NSInteger)radius;
- (void)setStrongBackgroundBlur;
- (void)setLightBackgroundBlur;

@end
