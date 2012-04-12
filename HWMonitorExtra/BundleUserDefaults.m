//
//  BundleUserDefaults.m
//
//  Created by John Chang on 6/15/07.
//  This code is Creative Commons Public Domain.  You may use it for any purpose whatsoever.
//  http://creativecommons.org/licenses/publicdomain/
//

#import "BundleUserDefaults.h"


@implementation BundleUserDefaults

- (id) initWithPersistentDomainName:(NSString *)domainName
{
	if ((self = [super init]))
	{
		_applicationID = [domainName copy];
		_registrationDictionary = nil;
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];
	}
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	/*[_applicationID release];
	[_registrationDictionary release];
	[super dealloc];*/
}


- (void) _applicationWillTerminate:(NSNotification *)notification
{
	[self synchronize];
}


- (id)objectForKey:(NSString *)defaultName
{
	id value = (__bridge id)CFPreferencesCopyAppValue((__bridge CFStringRef)defaultName, (__bridge CFStringRef)_applicationID);
	if (value == nil)
		value = [_registrationDictionary objectForKey:defaultName];
	return value;
}

- (void)setObject:(id)value forKey:(NSString *)defaultName
{
	CFPreferencesSetAppValue((__bridge CFStringRef)defaultName, (__bridge CFPropertyListRef)value, (__bridge CFStringRef)_applicationID);
}

- (void)removeObjectForKey:(NSString *)defaultName
{
	CFPreferencesSetAppValue((__bridge CFStringRef)defaultName, NULL, (__bridge CFStringRef)_applicationID);
}


- (void)registerDefaults:(NSDictionary *)registrationDictionary
{
	//[_registrationDictionary release];
	_registrationDictionary = registrationDictionary;
}


- (BOOL)synchronize
{
	return CFPreferencesSynchronize((__bridge CFStringRef)_applicationID, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
}

@end
