//
//  HWMonitorView.m
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorView.h"
#import "SystemUIPlugin.h"

@implementation HWMonitorView

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
    
    if (!self || !item)
        return nil;
    
    statusItem = item;
    
    isMenuExtra = [statusItem respondsToSelector:@selector(drawMenuBackground:)]; 
    
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
    NSImage * icon = nil;
    
    if (isMenuDown)
        icon = alternateImage;
    else
        icon = image;
    
    if (!monitor && !icon)
        return;
        
    if (isMenuExtra) {
        id menuExtra = (id)statusItem;
        [menuExtra drawMenuBackground:YES];
    }
    else [statusItem drawStatusBarBackgroundInRect:rect withHighlight:isMenuDown];
    
    if (icon)
        [icon drawAtPoint:NSMakePoint(isMenuExtra ? 0 : 3, 2) fromRect:NSMakeRect(0, 0, [icon size].width, [icon size].height) operation:NSCompositeSourceOver fraction:1.0];
    
    int size = (icon ? (isMenuExtra ? 0 : 3) + [icon size].width + 3 : 1);
    
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
            
            [title addAttribute:NSForegroundColorAttributeName value:(isMenuDown ? [NSColor whiteColor] : valueColor) range:NSMakeRange(0,[title length])];
            
            if (!isMenuDown && useShadowEffect) 
                [title addAttribute:NSShadowAttributeName value:shadow range:NSMakeRange(0,[title length])];
            
            if (drawValuesInRow) {
                [title addAttribute:NSFontAttributeName value:bigFont range:NSMakeRange(0, [title length])];
                
                [title drawAtPoint:NSMakePoint(size, ([self frame].size.height - [title size].height) / 2)];
                
                size = size + [title size].width + 3;
            }
            else {
                [title addAttribute:NSFontAttributeName value:smallFont range:NSMakeRange(0, [title length])];
                
                int row = i % 2;
                
                [title drawAtPoint:NSMakePoint(size, [favorites count] == 1 ? ([self frame].size.height - [title size].height) / 2 + 1 : row == 0 ? [self frame].size.height / 2 - 1: [self frame].size.height / 2 - [title size].height + 2)];
                
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
    
    size += isMenuExtra ? -2 : 0;
    
    [self setFrameSize:NSMakeSize(size, [self frame].size.height)];
    //snow leopard icon & text problem        
    [statusItem setLength:([self frame].size.width)];
}

- (void)mouseDown:(NSEvent *)event
{
    if (!isMenuExtra)
        [statusItem popUpStatusItemMenu:[statusItem menu]];
}

- (void)menuWillOpen:(NSMenu *)menu {
    [self setIsMenuDown:YES];
    [self setNeedsDisplay:YES];
}

- (void)menuDidClose:(NSMenu *)menu {
    [self setIsMenuDown:NO];
    [self setNeedsDisplay:YES];
}

@end
