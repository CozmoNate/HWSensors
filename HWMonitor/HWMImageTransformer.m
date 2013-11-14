//
//  HWMImageTransformer.m
//  HWMonitor
//
//  Created by Kozlek on 13.11.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMImageTransformer.h"

@implementation HWMImageTransformer

+ (Class)transformedValueClass
{
    return [NSData class];
}

+ (BOOL)allowsReverseTransformation
{
    return YES;
}

- (id)transformedValue:(id)value
{
    if (value == nil)
        return nil;

    if ([value isKindOfClass:[NSData class]])
        return value;

    if ([value isKindOfClass:[NSImage class]]) {
        return [(NSImage*)value TIFFRepresentation];
    }

    return nil;
}

- (id)reverseTransformedValue:(id)value
{
    return [[NSImage alloc] initWithData:(NSData *)value];
}

@end
