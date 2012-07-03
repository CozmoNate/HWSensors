//
//  NSString.h
//  HWSensors
//
//  Created by kozlek on 18/02/12.
//
//  Made by John Muchow on July 26, 2010
//
//  Original code http://iphonedevelopertips.com/cocoa/truncate-an-nsstring-and-append-an-ellipsis-respecting-the-font-size.html
//

//#import <Foundation/Foundation.h>

@interface NSString (TruncateToWidth)

- (NSString*)stringByTruncatingToWidth:(CGFloat)width withFont:(NSFont *)font;

@end
