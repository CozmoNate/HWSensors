//
//  BundleUserDefaults.h
//
//  Created by John Chang on 6/15/07.
//  This code is Creative Commons Public Domain.  You may use it for any purpose whatsoever.
//  http://creativecommons.org/licenses/publicdomain/
//

#import <Cocoa/Cocoa.h>


@interface BundleUserDefaults : NSUserDefaults {
	NSString * _applicationID;
	NSDictionary * _registrationDictionary;
}

- (id) initWithPersistentDomainName:(NSString *)domainName;

@end


@interface NSUserDefaultsController (SetDefaults)
- (void) _setDefaults:(NSUserDefaults *)defaults;
@end
