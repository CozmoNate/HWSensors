//
//  NSString.h
//  HWSensors
//
//  Created by Natan Zalkin on 18/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//
//  Original code http://iphonedevelopertips.com/cocoa/truncate-an-nsstring-and-append-an-ellipsis-respecting-the-font-size.html
//

#import <Foundation/Foundation.h>

@interface NSString (TruncateToWidth)

- (NSString*)stringByTruncatingToWidth:(CGFloat)width withFont:(NSFont *)font;

@end
