//
//  HWMEngineHelper.m
//  HWMonitor
//
//  Created by Kozlek on 05.01.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "HWMEngineHelper.h"

@implementation HWMEngineHelper

+ (NSIndexSet*)additionsFromItemsList:(NSArray*)newItems toItemsList:(NSArray*)oldItems
{
    NSMutableArray *new = [newItems mutableCopy];
    NSMutableArray *old = [oldItems mutableCopy];

    NSMutableIndexSet *additions = [[NSMutableIndexSet alloc] initWithIndexesInRange:NSMakeRange(0, new.count)];

    NSUInteger counter = 0;

    while (new.count && old.count) {

        id newObject = [new objectAtIndex:0];
        id oldObject = [old objectAtIndex:0];

        if ([oldObject isEqualTo:newObject]) {
            [additions removeIndex:counter];
            [old removeObjectAtIndex:0];
        }
        else {
            NSUInteger oldIndex = [old indexOfObject:newObject];

            if (oldIndex != NSNotFound) {
                [additions removeIndex:counter];
                [old removeObjectAtIndex:oldIndex];
            }
        }

        [new removeObjectAtIndex:0];

        counter++;
    }

    return additions;
}

+ (NSIndexSet*)deletionsFromItemsList:(NSArray*)oldItems newItemsList:(NSArray*)newItems
{
    NSMutableArray *new = [newItems mutableCopy];
    NSMutableArray *old = [oldItems mutableCopy];

    NSMutableIndexSet *deletions = [[NSMutableIndexSet alloc] initWithIndexesInRange:NSMakeRange(0, old.count)];

    NSUInteger counter = 0;

    while (new.count && old.count) {

        id newObject = [new objectAtIndex:0];
        id oldObject = [old objectAtIndex:0];

        if ([oldObject isEqualTo:newObject]) {
            [deletions removeIndex:counter];
            [new removeObjectAtIndex:0];
        }
        else {
            NSUInteger newIndex = [new indexOfObject:oldObject];

            if (newIndex != NSNotFound) {
                [deletions removeIndex:counter];
                [new removeObjectAtIndex:newIndex];
            }
        }

        [old removeObjectAtIndex:0];

        counter++;
    }
    
    return deletions;
}

+ (void)compareItemsList:(NSArray*)oldItems toItemsList:(NSArray*)newItems additions:(NSIndexSet**)additions deletions:(NSIndexSet**)deletions movedFrom:(NSIndexSet**)movedFrom movedTo:(NSIndexSet**)movedTo
{
    NSMutableIndexSet *removed = [[NSMutableIndexSet alloc] init];
    NSMutableIndexSet *added = [[NSMutableIndexSet alloc] init];
    NSMutableIndexSet *from = [[NSMutableIndexSet alloc] init];
    NSMutableIndexSet *to = [[NSMutableIndexSet alloc] init];

    NSMutableIndexSet *oldCounter = oldItems && oldItems.count ? [[NSMutableIndexSet alloc] initWithIndexesInRange:NSMakeRange(0, oldItems.count)] : [[NSMutableIndexSet alloc] init];
    NSMutableIndexSet *newCounter = newItems && newItems.count ? [[NSMutableIndexSet alloc] initWithIndexesInRange:NSMakeRange(0, newItems.count)] : [[NSMutableIndexSet alloc] init];

    while (oldCounter.count || newCounter.count) {

        if (!oldCounter.count) {
            // Add new objects
            [added addIndexes:newCounter];
            break;
        }

        if (!newCounter.count) {
            // Remove old objects
            [removed addIndexes:oldCounter];
            break;
        }

        id oldObject = [oldItems objectAtIndex:[oldCounter firstIndex]];
        id newObject = [newItems objectAtIndex:[newCounter firstIndex]];

        if ([oldObject isEqualTo:newObject]) {
            // Object not changed
            [oldCounter removeIndex:[oldCounter firstIndex]];
            [newCounter removeIndex:[newCounter firstIndex]];
        }
        else {
            NSUInteger newIndex = [newItems indexOfObject:oldObject];

            if (newIndex == NSNotFound) {
                // Object deleted
                [removed addIndex:[oldCounter firstIndex]];
                [oldCounter removeIndex:[oldCounter firstIndex]];
            }
            else {
                NSUInteger oldIndex = [oldItems indexOfObject:newObject];

                if (oldIndex == NSNotFound) {
                    // Object added
                    [added addIndex:[newCounter firstIndex]];
                    [newCounter removeIndex:[newCounter firstIndex]];
                    continue;
                }
                else {
                    // Object relocated
                    if (oldIndex <= newIndex) {
                        [from addIndex:oldCounter.firstIndex];
                        [to addIndex:newIndex];
                    }
                    else {
                        [from addIndex:oldIndex];
                        [to addIndex:newCounter.firstIndex];
                    }

                    [oldCounter removeIndex:oldCounter.firstIndex];
                    [oldCounter removeIndex:oldIndex];
                    [newCounter removeIndex:newCounter.firstIndex];
                    [newCounter removeIndex:newIndex];
                }
            }
        }
    }

    *deletions = removed;
    *additions = added;
    *movedFrom = from;
    *movedTo = to;
}

@end
