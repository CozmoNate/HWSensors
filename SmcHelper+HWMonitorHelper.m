//
//  SmcHelper+HWMonitorHelper.m
//  HWMonitor
//
//  Created by Kozlek on 22/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

// Based on code from https://github.com/atnan/SMJobBlessXPC

#import "SmcHelper+HWMonitorHelper.h"

#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

#import "SmcHelper.h"
#import "smc.h"
#import "FakeSMCDefinitions.h"

#define kSmcHelperLabel "org.hwsensors.HWMonitorHelper"
#define kSmcHelperCommandWriteNumber 1

@implementation SmcHelper (HWMonitorHelper)

+ (void)privilegedWriteNumericKey:(NSString*)key value:(NSNumber*)value
{
    if (![self blessHelperWithLabel:@kSmcHelperLabel])
        return;

    xpc_connection_t xpc_connection = xpc_connection_create_mach_service(kSmcHelperLabel, NULL, XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);

    xpc_connection_set_event_handler(xpc_connection, ^(xpc_object_t event) {
        xpc_type_t type = xpc_get_type(event);
        if (type == XPC_TYPE_ERROR) {
            if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
                NSLog(@"XPC connection interupted");
            }
            else if (event == XPC_ERROR_CONNECTION_INVALID) {
                NSLog(@"XPC connection invalid, releasing");
                xpc_release(xpc_connection);
            }
            else {
                NSLog(@"Unexpected XPC connection error");
            }
        }
        else {
            NSLog(@"Unexpected XPC connection event");
        }
    });

    xpc_connection_resume(xpc_connection);

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);

    xpc_dictionary_set_int64(message, "command", kSmcHelperCommandWriteNumber);
    xpc_dictionary_set_string(message, "key", key.UTF8String);
    xpc_dictionary_set_int64(message, "value", value.longLongValue);

    NSLog(@"Sending request to helper");

    xpc_connection_send_message_with_reply(xpc_connection, message, dispatch_get_main_queue(), ^(xpc_object_t event) {
        IOReturn result = (IOReturn)xpc_dictionary_get_int64(event, "result");
        NSLog(@"Received response from helper: %d", result);
    });
}

+ (BOOL)blessHelperWithLabel:(NSString *)label
{
    CFErrorRef localError = NULL;
	BOOL result = NO;

    CFDictionaryRef existingJob = SMJobCopyDictionary(kSMDomainSystemLaunchd, (__bridge CFStringRef)(label));

    if (existingJob) {
        NSDictionary *dictionary = (__bridge NSDictionary *)(existingJob);
        if ([[NSFileManager defaultManager] fileExistsAtPath:dictionary[@"ProgramArguments"][0]]) {
            dictionary = 0;
            CFRelease(existingJob);
            return true;
        }
    }

	AuthorizationItem authItems[2]  = {{ kSMRightBlessPrivilegedHelper, 0, NULL, 0 }, { kSMRightModifySystemDaemons, 0, NULL, 0 }};
	AuthorizationRights authRights	= { 1, authItems };
	AuthorizationFlags flags		=	kAuthorizationFlagDefaults				|
    kAuthorizationFlagInteractionAllowed	|
    kAuthorizationFlagPreAuthorize			|
    kAuthorizationFlagExtendRights;

	AuthorizationRef authRef = NULL;

	/* Obtain the right to install privileged helper tools (kSMRightBlessPrivilegedHelper). */
	OSStatus status = AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef);

	if (status != errAuthorizationSuccess) {
        NSLog(@"AuthorizationCreate() failed with error code %d", (int)status);
        return false;
	} else {
        /*if (helperIsAlreadyInstalled) {

            SMJobRemove(kSMDomainSystemLaunchd, (__bridge CFStringRef)(label), authRef, true, &localError);

            if (localError) {
                NSLog(@"SMJobRemove() failed with error %@", localError);
                CFRelease(localError);
                return false;
            }
        }*/

		result = SMJobBless(kSMDomainSystemLaunchd, (__bridge CFStringRef)label, authRef, (CFErrorRef*)&localError);

        if (localError) {
            NSLog(@"SMJobBless() failed with error %@", localError);
            CFRelease(localError);
            return false;
        }
	}

	return result;
}

@end
