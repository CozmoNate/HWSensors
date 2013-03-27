//
//  SensorsTableView.m
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "SensorsTableView.h"

@implementation SensorsTableView

//-(void)resetCursorRects
//{
//    [self discardCursorRects];
//    [self addCursorRect:[self bounds] cursor:[NSCursor openHandCursor]];
//}

//-(void)mouseDown:(NSEvent *)theEvent
//{
//    [super mouseDown:theEvent];
//    [[NSCursor closedHandCursor] set];
//}
//
//-(void)mouseUp:(NSEvent *)theEvent
//{
//    [super mouseUp:theEvent];
//    [[NSCursor openHandCursor] set];
//}

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
    
    NSPoint windowPoint = [self.window mouseLocationOutsideOfEventStream];
    NSPoint localPoint = [self convertPoint:windowPoint fromView:nil];
    
    id appController = self.delegate;
    
    if ([appController currentItemDragOperation] & NSDragOperationDelete ||
        ([appController hasDraggedFavoriteItem] && ![self mouse:localPoint inRect:[self visibleRect]])) {

        [appController setCurrentItemDragOperation:NSDragOperationDelete];
        
        [[NSCursor disappearingItemCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:NO];
    }
    else if ([appController currentItemDragOperation] & NSDragOperationPrivate) {
        [[NSCursor operationNotAllowedCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:YES];
    }
    else if ([appController currentItemDragOperation] & NSDragOperationCopy) {
        [[NSCursor dragCopyCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:NO];
    }
    else {
        [[NSCursor closedHandCursor] set];
        [session setAnimatesToStartingPositionsOnCancelOrFail:YES];
    }
}

@end
