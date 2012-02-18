//
//  NSString.h
//  HWSensors
//
//  Created by Natan Zalkin on 18/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (TruncateToWidth)

- (NSString*)stringByTruncatingToWidth:(CGFloat)width withFont:(NSFont *)font;

@end
