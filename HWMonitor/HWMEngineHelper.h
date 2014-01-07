//
//  HWMEngineHelper.h
//  HWMonitor
//
//  Created by Kozlek on 05.01.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface HWMEngineHelper : NSObject

+ (NSIndexSet*)additionsFromItemsList:(NSArray*)newItems toItemsList:(NSArray*)oldItems;
+ (NSIndexSet*)deletionsFromItemsList:(NSArray*)oldItems newItemsList:(NSArray*)newItems;
+ (void)compareItemsList:(NSArray*)oldItems toItemsList:(NSArray*)newItems additions:(NSIndexSet**)additions deletions:(NSIndexSet**)deletions movedFrom:(NSIndexSet**)movedFrom movedTo:(NSIndexSet**)movedTo;

@end
