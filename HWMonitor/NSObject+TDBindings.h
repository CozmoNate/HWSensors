//
//  NSObject+TDBindings.h
//  HWMonitor
//
//  Created by Kozlek on 01/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

// Got from http://tomdalling.com/blog/cocoa/implementing-your-own-cocoa-bindings/

#import <Foundation/Foundation.h>

@interface NSObject (TDBindings)

-(void) propagateValue:(id)value forBinding:(NSString*)binding;

@end
