//
//  NSWindow+BackgroundGaussianBlur.m
//  HWMonitor
//
//  Created by Kozlek on 15.03.14.
//  Copyright (c) 2014 Guilherme Rambo, kozlek. All rights reserved.
//

// Original article: http://stackoverflow.com/questions/19575642/how-to-use-cifilter-on-nswindow-in-osx-10-9-mavericks

#import "NSWindow+BackgroundBlur.h"
#import <objc/runtime.h>

typedef int CGSConnectionID;;
CG_EXTERN CGSConnectionID CGSMainConnectionID(void);
typedef CGError CGSSetWindowBackgroundBlurRadiusFunction(CGSConnectionID cid, NSInteger wid, NSUInteger blur);
CGSSetWindowBackgroundBlurRadiusFunction* GetCGSSetWindowBackgroundBlurRadiusFunction(void);

static NSString *const kApplicationServicesFramework = @"/System/Library/Frameworks/ApplicationServices.framework";

static void *GetFunctionByName(NSString *library, char *func) {
    CFBundleRef bundle;
    CFURLRef bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, (CFStringRef) library, kCFURLPOSIXPathStyle, true);
    CFStringRef functionName = CFStringCreateWithCString(kCFAllocatorDefault, func, kCFStringEncodingASCII);
    bundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);
    void *f = NULL;
    if (bundle) {
        f = CFBundleGetFunctionPointerForName(bundle, functionName);
        CFRelease(bundle);
    }
    CFRelease(functionName);
    CFRelease(bundleURL);
    return f;
}

CGSSetWindowBackgroundBlurRadiusFunction* GetCGSSetWindowBackgroundBlurRadiusFunction(void) {
    static BOOL tried = NO;
    static CGSSetWindowBackgroundBlurRadiusFunction *function = NULL;
    if (!tried) {
        function  = GetFunctionByName(kApplicationServicesFramework,
                                      "CGSSetWindowBackgroundBlurRadius");
        tried = YES;
    }
    return function;
}

@implementation NSWindow (BackgroundBlur)

- (NSInteger)backgroundBlurRadius
{
    return [objc_getAssociatedObject(self, @selector(backgroundBlurRadius)) integerValue];
}

- (void)setBackgroundBlurRadius:(NSInteger)radius
{
    NSInteger oldRadius = [objc_getAssociatedObject(self, @selector(backgroundBlurRadius)) integerValue];

    if (oldRadius != radius) {

        CGSConnectionID connection = CGSMainConnectionID();
        if (connection) {
            
            CGSSetWindowBackgroundBlurRadiusFunction* function = GetCGSSetWindowBackgroundBlurRadiusFunction();
            if (function) {
                if (0 == function(connection, [self windowNumber], (int)radius)) {
                    objc_setAssociatedObject(self, @selector(backgroundBlurRadius), [NSNumber numberWithInteger:radius], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
                }
            }
        }
    }
}

- (void)setHeavyBackgroundBlur
{
    [self setBackgroundBlurRadius:48];
}

- (void)setLightBackgroundBlur
{
    [self setBackgroundBlurRadius:16];
}

@end
