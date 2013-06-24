//
//  WindowFilter.m
//  HWMonitor
//
//  Created by Kozlek on 06/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "WindowFilter.h"

@implementation WindowFilter

- (id)initWithWindow:(NSWindow *)window name:(NSString*)filterName andOptions:(NSDictionary *)filterOptions
{
    self = [super init];
    
    if (self) {
        _windowNumber = [window windowNumber];
        CGSConnectionID connection = CGSMainConnectionID();
        CGSNewCIFilterByName(connection, (__bridge CFStringRef)filterName, &_filterRef);
        
        if (_filterRef) {
            CGSSetCIFilterValuesFromDictionary(connection, _filterRef, (__bridge  CFDictionaryRef)filterOptions);
            CGSAddWindowFilter(connection, _windowNumber, _filterRef, /*0x00003001*/1);
        }
    }
    
    return self;
}

- (void)setFilterOptions:(NSDictionary*)filterOptions
{
    if (!_filterRef)
        return;
    
    CGSConnectionID connection = CGSMainConnectionID();
    CGSSetCIFilterValuesFromDictionary(connection, _filterRef, (__bridge CFDictionaryRef)filterOptions);
}

- (void)removeFilterFromWindow
{
    if (!_filterRef)
        return;
    
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
