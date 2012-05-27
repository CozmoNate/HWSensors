//
//  HWMonitorExtraView.m
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import "HWMonitorCustomView.h"

@implementation HWMonitorCustomView

@synthesize image;
@synthesize alternateImage;
@synthesize isMenuDown;
@synthesize monitor;
@synthesize favorites;

@synthesize drawValuesInRow;
@synthesize useShadowEffect;

- initWithFrame:(NSRect)rect statusItem:(NSStatusItem*)item
{
    self = [super initWithFrame:rect];
    
    if( !self )
        return nil;
    
    statusItem = item;
    
    smallFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0f];
    bigFont = [NSFont fontWithName:@"Lucida Grande Bold" size:10.0f];
    
    shadow = [[NSShadow alloc] init];
    
    [shadow setShadowColor:[NSColor colorWithCalibratedWhite:1 alpha:0.55]];
    [shadow setShadowOffset:CGSizeMake(0, -1.0)];
    [shadow setShadowBlurRadius:1.0];
    
    return self;
}

- (void)drawRect:(NSRect)rect
{   
    [super drawRect:rect]; // not sure about this...
    
    BOOL down = isMenuDown;
    
    NSImage * icon = nil;
    
    if (down)
        icon = alternateImage;
    else
        icon = image;
    
    if (!monitor && !icon)
        return;
        
    if (down) 
        [statusItem drawStatusBarBackgroundInRect:rect withHighlight:YES];
    
    if (icon)
        [icon drawAtPoint:NSMakePoint(3,2) fromRect:NSMakeRect(0,0,14,19) operation:NSCompositeSourceOver fraction:1.0];
    
    int size = (icon ? 14 + 3 : 1);
    
    if (monitor && [[monitor sensors] count] > 0) {
        
        int lastWidth = 0;
        
        for (int i = 0; i < [favorites count]; i++) {
            
            HWMonitorSensor *sensor = [favorites objectAtIndex:i];
            
            if (!sensor)
                continue;

            NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:[sensor value]];
            
            NSColor *valueColor;
            
            switch ([sensor level]) {
                    /*case kHWSensorLevelDisabled:
                     break;
                     
                     case kHWSensorLevelNormal:
                     break;*/
                    
                case kHWSensorLevelModerate:
                    valueColor = [NSColor colorWithCalibratedRed:0.8f green:0.35f blue:0.035 alpha:1.0f];
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
            
            if (!down && useShadowEffect) 
                [title addAttribute:NSShadowAttributeName value:shadow range:NSMakeRange(0,[title length])];
            
            if (drawValuesInRow) {
                [title addAttribute:NSFontAttributeName value:bigFont range:NSMakeRange(0, [title length])];
                
                [title drawAtPoint:NSMakePoint(size, ([self frame].size.height - [title size].height) / 2)];
                
                size = size + [title size].width + 3;
            }
            else {
                [title addAttribute:NSFontAttributeName value:smallFont range:NSMakeRange(0, [title length])];
                
                int row = i % 2;
                
                [title drawAtPoint:NSMakePoint(size, [favorites count] == 1 ? ([self frame].size.height - [title size].height) / 2 + 1 : row == 0 ? [self frame].size.height / 2 - 1 : [self frame].size.height / 2 - [title size].height + 2)];
                
                int width = [title size].width + 4;
                
                if (row == 1) {
                    width = width > lastWidth ? width : lastWidth;
                    size = size + width;
                }
                else if (i == [favorites count] - 1) {
                    size = size + width;
                }
                
                lastWidth = width;
            }
        }
    }
    
    [self setFrameSize:NSMakeSize(size, [self frame].size.height)];
    //snow leopard icon & text problem        
    [statusItem setLength:([self frame].size.width)];
}

- (void)mouseDown:(NSEvent *)event
{ 
    [self setNeedsDisplay:YES];
    [statusItem popUpStatusItemMenu:[statusItem menu]];
}

- (void)setIsMenuDown:(BOOL)down 
{
    isMenuDown = down;

   [self setNeedsDisplay:YES];
}

- (BOOL)isMenuDown
{
    return isMenuDown;
}

@end
