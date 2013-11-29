//
//  SmcHelper.h
//  HWMonitor
//
//  Created by Kozlek on 28/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SmcHelper : NSObject

+ (BOOL)isValidIntegerSmcType:(NSString *)type;
+ (BOOL)isValidFloatingSmcType:(NSString *)type;
+ (float)decodeNumericValueFromBuffer:(void*)data length:(NSUInteger)length type:(const char *)type;

@end
