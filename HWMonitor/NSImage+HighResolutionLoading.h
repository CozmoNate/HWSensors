//
//  NSImage+HighResolutionLoading.h
//  HWMonitor
//
//  Created by Kozlek on 18.01.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSImage (HighResolutionLoading)

+(NSImage*)loadImageNamed:(NSString*)name ofType:(NSString*)type;
+(NSImage*)loadImageNamed:(NSString*)name ofType:(NSString*)type useHighResolutionArtwork:(BOOL)useHighResolution;

@end
