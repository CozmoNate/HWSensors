//
//  WindowFilter.h
//  HWMonitor
//
//  Created by Kozlek on 06/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

typedef int* CGSWindowFilterRef;
typedef int CGSConnectionID;
typedef int CGSWindowID;

@interface WindowFilter : NSObject
{
@private
    CGWindowID         _windowNumber;
    CGSWindowFilterRef  _filterRef;
}

- (id)initWithWindow:(NSWindow *)window name:(NSString*)filterName andOptions:(NSDictionary *)filterOptions;
- (void)removeFilterFromWindow;

@end
