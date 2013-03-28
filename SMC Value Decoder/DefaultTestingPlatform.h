//
//  SMC_Value_Decoder.h
//  SMC Value Decoder
//
//  Created by kozlek on 09.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <SenTestingKit/SenTestingKit.h>

@interface SMC_Value_Decoder : SenTestCase

@property (nonatomic, strong) NSString* name;
@property (nonatomic, strong) NSString* type;
@property (nonatomic, assign) NSUInteger group;
@property (nonatomic, strong) NSString* title;
@property (nonatomic, strong) NSData* data;

@property (readonly) NSNumber* rawValue;
@property (readonly) NSString* formattedValue;

@property (readonly) BOOL levelHasBeenChanged;
@property (readonly) BOOL valueHasBeenChanged;

@end
