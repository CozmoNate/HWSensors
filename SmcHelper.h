//
//  SmcHelper.h
//  HWMonitor
//
//  Created by Kozlek on 28/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SmcHelper : NSObject

+ (int)getIndexFromHexChar:(char)c;
+ (BOOL)isValidIntegerSmcType:(NSString *)type;
+ (BOOL)isValidFloatingSmcType:(NSString *)type;
+ (NSNumber*)decodeNumericValueFromBuffer:(void*)data length:(NSUInteger)length type:(const char *)type;
+ (BOOL)encodeNumericValue:(NSNumber*)value length:(NSUInteger)length type:(const char *)type outBuffer:(void*)outBuffer;

@end
