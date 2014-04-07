//
//  NSString+IOReturnDescription.h
//  HWMonitor
//
//  Created by Kozlek on 03.04.14.
//  Copyright (c) 2014 emeraldo.cs. All rights reserved.
//

// Based on code from http://stackoverflow.com/questions/3887309/mapping-iokit-ioreturn-error-code-to-string

#import <Foundation/Foundation.h>

@interface NSString (IOReturnDescription)

+(NSString*)stringWithDescriptionForReturnCode:(IOReturn)errorVal;

@end
