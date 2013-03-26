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

-(void)draggedImage:(NSImage *)image beganAt:(NSPoint)screenPoint
{
    [[NSCursor closedHandCursor] set];
}

-(void)draggedImage:(NSImage *)image endedAt:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    [[NSCursor openHandCursor] set];
}

- (void)draggedImage:(NSImage *)image movedTo:(NSPoint)screenPoint
{
    NSPoint windowPoint = [self.window mouseLocationOutsideOfEventStream];
    NSPoint localPoint = [self convertPoint:windowPoint fromView:nil];
    
    id appController = self.delegate;
    
    if ([appController currentItemDragOperation] == NSDragOperationDelete || ([appController hasDraggedFavoriteItem] && ![self mouse:localPoint inRect:[self visibleRect]])) {
        [[NSCursor disappearingItemCursor] set];
        [appController setCurrentItemDragOperation:NSDragOperationDelete];
    }
    else if ([appController currentItemDragOperation] == NSDragOperationPrivate) {
        [[NSCursor operationNotAllowedCursor] set];
    }
    else if ([appController currentItemDragOperation] == NSDragOperationCopy) {
        [[NSCursor dragCopyCursor] set];
    }
    else {
        [[NSCursor closedHandCursor] set];
    }
}

@end
