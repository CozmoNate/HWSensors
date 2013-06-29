//
//  WindowFilter.m
//  HWMonitor
//
//  Created by Kozlek on 06/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

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
