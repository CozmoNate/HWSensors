//
//  SmcHelper.h
//  HWMonitor
//
//  Created by Kozlek on 28/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "smc.h"

@interface SmcHelper : NSObject

+ (int)getIndexFromHexChar:(char)c;
+ (BOOL)isValidIntegerSmcType:(NSString *)type;
+ (BOOL)isValidFloatingSmcType:(NSString *)type;
+ (NSNumber*)decodeNumericValueFromBuffer:(void*)data length:(NSUInteger)length type:(const char *)type;
+ (BOOL)encodeNumericValue:(NSNumber*)value length:(NSUInteger)length type:(const char *)type outBuffer:(void*)outBuffer;

+ (NSNumber*)readNumericKey:(NSString*)key connection:(io_connect_t)connection;
+ (BOOL)writeNumericKey:(NSString*)key value:(NSNumber*)value connection:(io_connect_t)connection;

@end
