//
//  HWMValueFormatter.h
//  HWMonitor
//
//  Created by Kozlek on 16.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@class HWMConfiguration;

@interface HWMValueFormatter : NSObject

+ (NSString*)formattedValue:(NSNumber*)value usingRulesOfGroup:(NSNumber*)selector configuration:(HWMConfiguration*)configuration;

+ (NSString*)strippedValue:(NSNumber*)value usingRulesOfGroup:(NSNumber*)selector configuration:(HWMConfiguration*)configuration;

@end
