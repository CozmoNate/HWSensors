//
//  WindowFilter.h
//  HWMonitor
//
//  Created by Kozlek on 06/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

typedef int* CGSWindowFilterRef;
typedef int CGSConnectionID;
typedef long CGSWindowID;

extern CGSConnectionID CGSMainConnectionID(void);
extern CGError CGSNewCIFilterByName(CGSConnectionID cid, CFStringRef filterName, CGSWindowFilterRef *outFilter);
extern CGError CGSSetCIFilterValuesFromDictionary(CGSConnectionID cid, CGSWindowFilterRef filter, CFDictionaryRef filterValues);
extern CGError CGSReleaseCIFilter(CGSConnectionID cid, CGSWindowFilterRef filter);
extern CGError CGSAddWindowFilter(CGSConnectionID cid, CGSWindowID wid, CGSWindowFilterRef filter, int flags);
extern CGError CGSRemoveWindowFilter(CGSConnectionID cid, CGSWindowID wid, CGSWindowFilterRef filter);

@interface WindowFilter : NSObject
{
@private
    CGSWindowID         _windowNumber;
    CGSWindowFilterRef  _filterRef;
}

- (id)initWithWindow:(NSWindow *)window name:(NSString*)filterName andOptions:(NSDictionary *)filterOptions;
- (void)setFilterOptions:(NSDictionary*)filterOptions;
- (void)removeFilterFromWindow;

@end
