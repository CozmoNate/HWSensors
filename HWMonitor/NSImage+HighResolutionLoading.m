//
//  NSImage+HighResolutionLoading.m
//  HWMonitor
//
//  Created by Kozlek on 18.01.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "NSImage+HighResolutionLoading.h"

@implementation NSImage (HighResolutionLoading)

+(NSImage*)loadImageNamed:(NSString*)name ofType:(NSString*)type
{
    return [NSImage loadImageNamed:name ofType:type useHighResolutionArtwork:[[NSScreen mainScreen] backingScaleFactor] > 1.0f];
}

+(NSImage*)loadImageNamed:(NSString*)name ofType:(NSString*)type useHighResolutionArtwork:(BOOL)useHighResolution
{
    return [[NSBundle mainBundle] imageForResource:name];
    //return [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[NSString stringWithFormat:@"%@%@", name, useHighResolution ? @"@2x" : @""] ofType:type]];
}

@end
