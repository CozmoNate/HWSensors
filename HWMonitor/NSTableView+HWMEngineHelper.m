//
//  NSTableView+HWMEngineHelper.m
//  HWMonitor
//
//  Created by Kozlek on 12.01.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "NSTableView+HWMEngineHelper.h"

@implementation NSTableView (HWMEngineHelper)

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

-(void)updateWithObjectValues:(NSArray*)objectValues previousObjectValues:(NSArray*)oldObjectValues updateHeightOfTheRows:(BOOL)updateHeights withRemoveAnimation:(NSTableViewAnimationOptions)removeAnime insertAnimation:(NSTableViewAnimationOptions)insertAnime;
{
    NSIndexSet *inserted, *removed, *from, *to;

    [NSTableView compareItemsList:oldObjectValues toItemsList:objectValues additions:&inserted deletions:&removed movedFrom:&from movedTo:&to];

    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {

        [self removeRowsAtIndexes:removed withAnimation:removeAnime];
        [self insertRowsAtIndexes:inserted withAnimation:insertAnime];

        NSUInteger fromIndex = [from firstIndex];
        NSUInteger toIndex = [to firstIndex];
        while (fromIndex != NSNotFound && toIndex != NSNotFound) {
            [self moveRowAtIndex:fromIndex toIndex:toIndex];
            fromIndex = [from indexGreaterThanIndex:fromIndex];
            toIndex = [to indexGreaterThanIndex:toIndex];
        }

    } completionHandler:^{
        if (updateHeights) {
            [[NSOperationQueue mainQueue] addOperationWithBlock:^{
                [NSAnimationContext beginGrouping];
                //[[NSAnimationContext currentContext] setDuration:[[NSAnimationContext currentContext] duration]];
                [self noteHeightOfRowsWithIndexesChanged:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, objectValues.count)]];
                [NSAnimationContext endGrouping];
            }];
        }
    }];
}

-(void)updateWithObjectValues:(NSArray*)oldObjects previousObjectValues:(NSArray*)newObjects
{
    [self updateWithObjectValues:oldObjects previousObjectValues:newObjects
           updateHeightOfTheRows:YES
             withRemoveAnimation:NSTableViewAnimationSlideUp
                 insertAnimation:NSTableViewAnimationSlideDown];
}

@end
