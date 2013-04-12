//
//  WindowFilter.m
//  HWMonitor
//
//  Created by Kozlek on 06/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "WindowFilter.h"

extern CGSConnectionID CGSMainConnectionID(void);
extern CGError CGSNewCIFilterByName(CGSConnectionID cid, CFStringRef filterName, CGSWindowFilterRef *outFilter);
extern CGError CGSSetCIFilterValuesFromDictionary(CGSConnectionID cid, CGSWindowFilterRef filter, CFDictionaryRef filterValues);
extern CGError CGSReleaseCIFilter(CGSConnectionID cid, CGSWindowFilterRef filter);
extern CGError CGSAddWindowFilter(CGSConnectionID cid, CGSWindowID wid, CGSWindowFilterRef filter, int flags);
extern CGError CGSRemoveWindowFilter(CGSConnectionID cid, CGSWindowID wid, CGSWindowFilterRef filter);

@implementation WindowFilter

- (id)initWithWindow:(NSWindow *)window name:(NSString*)filterName andOptions:(NSDictionary *)filterOptions
{
    self = [super init];
    
    if (self) {
        _windowNumber = [window windowNumber];
        CGSConnectionID connection = CGSMainConnectionID();
        CGSNewCIFilterByName(connection, (__bridge CFStringRef)filterName, &_filterRef);
        CGSSetCIFilterValuesFromDictionary(connection, _filterRef, (__bridge  CFDictionaryRef)filterOptions);
        CGSAddWindowFilter(connection, _windowNumber, _filterRef, 1);
    }
    
    return self;
}

- (void)setFilterOptions:(NSDictionary*)filterOptions
{
    CGSConnectionID connection = CGSMainConnectionID();
    CGSSetCIFilterValuesFromDictionary(connection, _filterRef, (__bridge CFDictionaryRef)filterOptions);
}

- (void)removeFilterFromWindow
{
    CGSConnectionID connection = CGSMainConnectionID();
    CGSRemoveWindowFilter(connection, _windowNumber, _filterRef);
    CGSReleaseCIFilter(connection, _filterRef);
    _filterRef = 0;
}

-(void)dealloc
{
    [self removeFilterFromWindow];
}

@end
