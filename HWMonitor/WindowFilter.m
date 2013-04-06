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
        CGSNewConnection(NULL, &_connection);
        CGSNewCIFilterByName(_connection, (__bridge CFStringRef)filterName, &_filterRef);
        CGSSetCIFilterValuesFromDictionary(_connection, _filterRef, (__bridge CFDictionaryRef)filterOptions);
        CGSAddWindowFilter(_connection, _windowNumber, _filterRef, 1);
    }
    return self;
}

- (void)removeFilterFromWindow
{
    if (_filterRef && _windowNumber > -1) {
        CGSRemoveWindowFilter(_connection, _windowNumber, _filterRef);
        CGSReleaseCIFilter(_connection, _filterRef);
        CGSReleaseConnection(_connection);
        _filterRef = 0;
    }
}

-(void)dealloc
{
    [self removeFilterFromWindow];
}

@end
