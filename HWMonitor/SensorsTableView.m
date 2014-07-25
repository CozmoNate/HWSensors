//
//  SensorsTableView.m
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "SensorsTableView.h"

@implementation SensorsTableView

-(void)draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint
{
    [super draggingSession:session willBeginAtPoint:screenPoint];
    [[NSCursor closedHandCursor] set];
}

-(void)draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    [super draggingSession:session endedAtPoint:screenPoint operation:operation];
    [[NSCursor openHandCursor] set];
}

-(void)draggingSession:(NSDraggingSession *)session movedToPoint:(NSPoint)screenPoint
{
    //[super draggingSession:session movedToPoint:screenPoint];
    
    id viewController = self.delegate;
    
    if (!viewController || ![viewController respondsToSelector:@selector(currentItemDragOperation)])
        return;
    
    NSPoint windowPoint = [self.window mouseLocationOutsideOfEventStream];
    NSPoint localPoint = [self convertPoint:windowPoint fromView:nil];
    
    if ([viewController respondsToSelector:@selector(hasDraggedFavoriteItem)]) {
        if ([viewController currentItemDragOperation] != NSDragOperationDelete && [viewController hasDraggedFavoriteItem] && [self mouse:localPoint inRect:NSInsetRect([self visibleRect], -35, -35)] == NO) {
            [viewController setCurrentItemDragOperation:NSDragOperationDelete];
        }
        else if ([viewController currentItemDragOperation] == NSDragOperationDelete && [viewController hasDraggedFavoriteItem] && [self mouse:localPoint inRect:NSInsetRect([self visibleRect], -35, -35)] == YES) {
            [viewController setCurrentItemDragOperation:NSDragOperationNone];
        }
    }
    
    if ([viewController currentItemDragOperation] == NSDragOperationDelete /*&& [NSCursor currentCursor] != [NSCursor disappearingItemCursor]*/) {
        [[NSCursor disappearingItemCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:NO];
    }
    else if ([viewController currentItemDragOperation] == NSDragOperationPrivate /*&& [NSCursor currentCursor] != [NSCursor operationNotAllowedCursor]*/) {
        [[NSCursor operationNotAllowedCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:YES];
    }
    else if ([viewController currentItemDragOperation] == NSDragOperationCopy /*&& [NSCursor currentCursor] != [NSCursor dragCopyCursor]*/) {
        [[NSCursor dragCopyCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:NO];
    }
    else /*if ([NSCursor currentCursor] != [NSCursor closedHandCursor])*/ {
        [[NSCursor closedHandCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:YES];
    }
}

@end
