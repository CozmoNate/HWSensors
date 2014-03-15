//
//  NSWindow+BackgroundGaussianBlur.h
//  HWMonitor
//
//  Created by Kozlek on 15.03.14.
//  Copyright (c) 2014 Guilherme Rambo, kozlek. All rights reserved.
//

// Original article: http://stackoverflow.com/questions/19575642/how-to-use-cifilter-on-nswindow-in-osx-10-9-mavericks

#import <Cocoa/Cocoa.h>

@interface NSWindow (BackgroundBlurPrivate)

- (void)setBackgroundBlurRadius:(NSInteger)radius;

@end
