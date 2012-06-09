//
//  HWMonitorIcon.h
//  HWSensors
//
//  Created by kozlek on 09.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface HWMonitorIcon : NSObject

@property (readwrite, retain) NSString *name;
@property (readwrite, retain) NSImage *image;
@property (readwrite, retain) NSImage *alternateImage;

+(HWMonitorIcon*)iconWithName:(NSString*)aName image:(NSImage*)anImage alternateImage:(NSImage*)anAlternateImage;

@end
