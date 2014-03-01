//
//  NSObject+TDBindings.m
//  HWMonitor
//
//  Created by Kozlek on 01/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

// Got from http://tomdalling.com/blog/cocoa/implementing-your-own-cocoa-bindings/

#import "NSObject+TDBindings.h"

@implementation NSObject (TDBindings)

-(void) propagateValue:(id)value forBinding:(NSString*)binding
{
	NSParameterAssert(binding != nil);

	//WARNING: bindingInfo contains NSNull, so it must be accounted for
	NSDictionary* bindingInfo = [self infoForBinding:binding];
	if(!bindingInfo)
		return; //there is no binding

	//apply the value transformer, if one has been set
	NSDictionary* bindingOptions = [bindingInfo objectForKey:NSOptionsKey];
	if(bindingOptions){
		NSValueTransformer* transformer = [bindingOptions valueForKey:NSValueTransformerBindingOption];
		if(!transformer || (id)transformer == [NSNull null]){
			NSString* transformerName = [bindingOptions valueForKey:NSValueTransformerNameBindingOption];
			if(transformerName && (id)transformerName != [NSNull null]){
				transformer = [NSValueTransformer valueTransformerForName:transformerName];
			}
		}

		if(transformer && (id)transformer != [NSNull null]){
			if([[transformer class] allowsReverseTransformation]){
				value = [transformer reverseTransformedValue:value];
			} else {
				NSLog(@"WARNING: binding \"%@\" has value transformer, but it doesn't allow reverse transformations in %s", binding, __PRETTY_FUNCTION__);
			}
		}
	}

	id boundObject = [bindingInfo objectForKey:NSObservedObjectKey];
	if(!boundObject || boundObject == [NSNull null]){
		NSLog(@"ERROR: NSObservedObjectKey was nil for binding \"%@\" in %s", binding, __PRETTY_FUNCTION__);
		return;
	}

	NSString* boundKeyPath = [bindingInfo objectForKey:NSObservedKeyPathKey];
	if(!boundKeyPath || (id)boundKeyPath == [NSNull null]){
		NSLog(@"ERROR: NSObservedKeyPathKey was nil for binding \"%@\" in %s", binding, __PRETTY_FUNCTION__);
		return;
	}

	[boundObject setValue:value forKeyPath:boundKeyPath];
}

@end
