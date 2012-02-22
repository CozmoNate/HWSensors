//
//  HWMonitorExtraView.m
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import "HWMonitorView.h"

@implementation HWMonitorView

- (void)setImage:(NSImage*)newImage
{
    image = newImage;
}

- (void)setAlternateImage:(NSImage*)newAlternateImage
{
    altImage = newAlternateImage;
}

- (void)setTitles:(NSMutableArray*)newTitles
{
    titles = [[NSArray alloc] initWithArray:newTitles];
    
    [self setNeedsDisplay:YES];
}

- initWithFrame:(NSRect)rect menuExtra:m
{
    self = [super initWithFrame:rect];
    
    if( !self )
        return nil;
    
    menu = m;
    font = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0f];
    
    return self;
}

- (void)drawRect:(NSRect)rect
{
    NSImage * icon;
    
    [super drawRect:rect]; // not sure about this...
    
    BOOL down = [menu isMenuDown];
    
    if (down)
        icon = altImage;
    else
        icon = image;
    
    if (!titles && !icon)
        return;
        
    /*if (down)*/ [menu drawMenuBackground:YES];
    
    if (icon) {
        [icon drawAtPoint:NSMakePoint(0,2) fromRect:NSMakeRect(0,0,11,19) operation:NSCompositeSourceOver fraction:1.0];
    }
    
    int size = (icon ? 11 + 3 : 1);
    
    if (titles && [titles count] > 0) {
        
        int lastWidth = 0;
        
        for (int i = 0; i < [titles count]; i++) {
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:(NSString*)[titles objectAtIndex:i]];
            
            [title addAttribute:NSFontAttributeName value:font range:NSMakeRange(0, [title length])];
            [title addAttribute:NSForegroundColorAttributeName value:(down ? [NSColor whiteColor] : [NSColor blackColor]) range:NSMakeRange(0,[title length])];
            
            int row = i % 2;
            
            [title drawAtPoint:NSMakePoint(size, [titles count] == 1 ? 6 : row == 0 ? 10 : 1)];
            
            int width = [title size].width + 4;
            
            if (row == 1) {
                lastWidth = width > lastWidth ? width : lastWidth;
                size = size + lastWidth;
            }
            else if (i + 1 == [titles count]) {
                size = size + width;
            }
            
            lastWidth = width;
        }
    }
    
    [self setFrameSize:NSMakeSize(size, [self frame].size.height)];
    //snow leopard icon & text problem        
    [menu setLength:([self frame].size.width)];
}

@end
