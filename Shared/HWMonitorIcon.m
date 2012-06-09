//
//  HWMonitorIcon.m
//  HWSensors
//
//  Created by kozlek on 09.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorIcon.h"

@implementation HWMonitorIcon
@synthesize name;
@synthesize image;
@synthesize alternateImage;

+(HWMonitorIcon*)iconWithName:(NSString*)aName image:(NSImage*)anImage alternateImage:(NSImage*)anAlternateImage
{
    HWMonitorIcon*  me = [[HWMonitorIcon alloc] init];
    
    if (me) {
        [me setName:aName];
        [me setImage:anImage];
        [me setAlternateImage:anAlternateImage];
    }
    
    return me;
}

@end
