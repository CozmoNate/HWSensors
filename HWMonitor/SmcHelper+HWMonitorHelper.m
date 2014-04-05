//
//  SmcHelper+HWMonitorHelper.m
//  HWMonitor
//
//  Created by Kozlek on 04.04.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "SmcHelper+HWMonitorHelper.h"

#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

#import "FakeSMCDefinitions.h"
#import "SmcHelper.h"

#import "NSString+IOReturnDescription.h"

@implementation SmcHelper (HWMonitorHelper)

+ (BOOL)privilegedWriteNumericKey:(NSString*)key value:(NSNumber*)value
{
    if (![self blessHelperWithLabel:@kHWMonitorHelperServiceName])
        return FALSE;

    NSConnection *connection = [NSConnection connectionWithRegisteredName:@kHWMonitorHelperServiceName host:nil];

    HWMonitorHelper *proxy = (HWMonitorHelper *)[connection rootProxy];

    return [proxy writeNumericKey:key value:value];
}

+ (BOOL)blessHelperWithLabel:(NSString *)label
{
 	BOOL result = NO;

    NSDictionary *helperJobData = (NSDictionary*)CFBridgingRelease(SMJobCopyDictionary(kSMDomainSystemLaunchd, (__bridge CFStringRef)label));

    if (helperJobData) {
        NSURL *installedHelperURL = [NSURL fileURLWithPath:helperJobData[@"ProgramArguments"][0]];
        NSDictionary *installedHelperInfoPlist = (NSDictionary*)CFBridgingRelease(CFBundleCopyInfoDictionaryForURL((__bridge CFURLRef)installedHelperURL));
        NSInteger installedHelperVersion = [installedHelperInfoPlist[@"CFBundleVersion"] integerValue];

        NSURL *currentHelperToolURL = [[[NSBundle mainBundle] bundleURL] URLByAppendingPathComponent:@"Contents/Library/LaunchServices/org.hwsensors.HWMonitorHelper"];
        NSDictionary *currentHelperInfoPlist = (NSDictionary*)CFBridgingRelease(CFBundleCopyInfoDictionaryForURL( (__bridge CFURLRef)currentHelperToolURL));
        NSInteger currentHelperVersion = [currentHelperInfoPlist[@"CFBundleVersion"] integerValue];

        if (installedHelperVersion == currentHelperVersion) {
            return true;
        }
    }

    // Install helper tool
	AuthorizationItem authItems[2]  = {
        {kSMRightBlessPrivilegedHelper, 0, NULL, 0},
        {kSMRightModifySystemDaemons, 0, NULL, 0}
    };

	AuthorizationRights authRights	= {helperJobData ? 2 : 1, authItems};
	AuthorizationFlags flags		= kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed	|kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;

	AuthorizationRef authRef = NULL;

	/* Obtain the right to install privileged helper tools (kSMRightBlessPrivilegedHelper). */
	OSStatus status = AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef);

	if (status != errAuthorizationSuccess) {
        NSLog(@"AuthorizationCreate() failed with error code %d", (int)status);
        return false;
	}
    else {
        CFErrorRef localError = NULL;

        if (helperJobData) {

            SMJobRemove(kSMDomainSystemLaunchd, (__bridge CFStringRef)(label), authRef, true, &localError);

            if (localError) {
                NSLog(@"SMJobRemove() failed with error %@", localError);
                CFRelease(localError);
            }
        }

		SMJobBless(kSMDomainSystemLaunchd, (__bridge CFStringRef)(label), authRef, (CFErrorRef*)&localError);

        if (localError) {
            NSLog(@"SMJobBless() failed with error %@", localError);
            CFRelease(localError);
        }
	}
    
	return result;
}

@end
