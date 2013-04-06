//
//  WindowFilter.h
//  HWMonitor
//
//  Created by Kozlek on 06/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//


@interface WindowFilter : NSObject
{
@private
    long _windowNumber;
    long _connection;
    void *_filterRef;
}

- (id)initWithWindow:(NSWindow *)window name:(NSString*)filterName andOptions:(NSDictionary *)filterOptions;
- (void)removeFilterFromWindow;

@end
