//
//  SmcHelper.h
//  HWMonitor
//
//  Created by Kozlek on 28/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "smc.h"

#define SMC_TYPE_FPE2   "fpe2"
#define SMC_TYPE_FP2E   "fp2e"
#define SMC_TYPE_FP4C   "fp4c"
#define SMC_TYPE_CH8    "ch8*"
#define SMC_TYPE_SP78   "sp78"
#define SMC_TYPE_SP4B   "sp4b"
#define SMC_TYPE_FP5B   "fp5b"
#define SMC_TYPE_FP88   "fp88"
#define SMC_TYPE_UI8    "ui8"
#define SMC_TYPE_UI16   "ui16"
#define SMC_TYPE_UI32   "ui32"
#define SMC_TYPE_SI8   "si8"
#define SMC_TYPE_SI16   "si16"
#define SMC_TYPE_SI32   "si32"
#define SMC_TYPE_FLAG   "flag"
#define SMC_TYPE_FDS    "{fds"

#define SMC_TYPE_FPXX_SIZE  2
#define SMC_TYPE_SPXX_SIZE  2
#define SMC_TYPE_UI8_SIZE   1
#define SMC_TYPE_UI16_SIZE  2
#define SMC_TYPE_UI32_SIZE  4
#define SMC_TYPE_SI8_SIZE   1
#define SMC_TYPE_SI16_SIZE  2
#define SMC_TYPE_SI32_SIZE  4
#define SMC_TYPE_FLAG_SIZE  1

@interface SmcHelper : NSObject

+ (int)getIndexFromHexChar:(char)c;
+ (BOOL)isValidIntegerSmcType:(NSString *)type;
+ (BOOL)isValidFloatingSmcType:(NSString *)type;
+ (NSNumber*)decodeNumericValueFromBuffer:(void*)data length:(NSUInteger)length type:(const char *)type;
+ (BOOL)encodeNumericValue:(NSNumber*)value length:(NSUInteger)length type:(const char *)type outBuffer:(void*)outBuffer;

+ (NSNumber*)readNumericKey:(NSString*)key connection:(io_connect_t)connection;
+ (BOOL)writeNumericKey:(NSString*)key value:(NSNumber*)value connection:(io_connect_t)connection;

@end
