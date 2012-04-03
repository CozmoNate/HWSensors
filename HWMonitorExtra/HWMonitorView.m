//
//  HWMonitorExtraView.m
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import "HWMonitorView.h"

@implementation HWMonitorView

@synthesize monitor;

- initWithFrame:(NSRect)rect menuExtra:m
{
    self = [super initWithFrame:rect];
    
    if( !self )
        return nil;
    
    menu = m;
    
    font = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0f];
    //font = [NSFont boldSystemFontOfSize:9.0f];
    
    shadow = [[NSShadow alloc] init];
    
    [shadow setShadowColor:[NSColor colorWithSRGBRed:1 green:1 blue:1 alpha:0.6]];
    [shadow setShadowOffset:CGSizeMake(0, -1.0)];
    [shadow setShadowBlurRadius:1.0];
    
    return self;
}

- (void)drawRect:(NSRect)rect
{   
    [super drawRect:rect]; // not sure about this...
    
    BOOL down = [menu isMenuDown];
    
    NSImage * icon = nil;
    
    if (down)
        icon = _alternateImage;
    else
        icon = _image;
    
    if (!monitor && !icon)
        return;
        
    /*if (down)*/ [menu drawMenuBackground:YES];
    
    if (icon) {
        [icon drawAtPoint:NSMakePoint(0,2) fromRect:NSMakeRect(0,0,11,19) operation:NSCompositeSourceOver fraction:1.0];
    }
    
    int size = (icon ? 11 + 3 : 1);
    
    if (monitor && [[monitor sensors] count] > 0) {
        
        int lastWidth = 0;
        
        NSEnumerator *enumerator = [[monitor sensors] objectEnumerator];
        
        HWMonitorSensor *sensor = nil;
        NSMutableArray *favorites = [[NSMutableArray alloc] init];
        
        while (sensor = (HWMonitorSensor*)[enumerator nextObject]) 
            if ([sensor favorite])
                [favorites addObject:sensor];
        
        for (int i = 0; i < [favorites count]; i++) {
            
            sensor = (HWMonitorSensor*)[favorites objectAtIndex:i];
            
            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:[sensor formattedValue]];
            
            [title addAttribute:NSFontAttributeName value:font range:NSMakeRange(0, [title length])];
            
            NSColor *valueColor;
            
            switch ([sensor level]) {
                    /*case kHWSensorLevelDisabled:
                     break;
                     
                     case kHWSensorLevelNormal:
                     break;*/
                    
                case kHWSensorLevelModerate:
                    valueColor = [NSColor orangeColor];
                    break;
                    
                case kHWSensorLevelHigh:
                case kHWSensorLevelExceeded:
                    valueColor = [NSColor redColor];
                    break;
                    
                default:
                    valueColor = [NSColor blackColor];
                    break;
            }
            
            [title addAttribute:NSForegroundColorAttributeName value:(down ? [NSColor whiteColor] : valueColor) range:NSMakeRange(0,[title length])];
            
            if (!down) 
                [title addAttribute:NSShadowAttributeName value:shadow range:NSMakeRange(0,[title length])];
            
            int row = i % 2;
            
            [title drawAtPoint:NSMakePoint(size, [favorites count] == 1 ? 6 : row == 0 ? 10 : 1)];
            
            int width = [title size].width + ([favorites count] == i + 1 ? 1 : 4);
            
            if (row == 1) {
                lastWidth = width > lastWidth ? width : lastWidth;
                size = size + lastWidth;
            }
            else if (i + 1 == [favorites count]) {
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
