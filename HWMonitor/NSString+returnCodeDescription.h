//
//  NSString+returnCodeDescription.h
//  HWMonitor
//
//  Created by Kozlek on 15.04.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (returnCodeDescription)

+(NSString*)stringFromReturnCode:(IOReturn)code;

@end
