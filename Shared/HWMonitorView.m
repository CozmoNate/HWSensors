//
//  HWMonitorView.m
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorView.h"
#import "HWMonitorIcon.h"

#import "SystemUIPlugin.h"

@implementation HWMonitorView

@synthesize image=_image;
@synthesize alternateImage=_alternateImage;
@synthesize isMenuDown=_isMenuDown;
@synthesize engine=_engine;
@synthesize favorites=_favorites;

@synthesize drawValuesInRow;
@synthesize useShadowEffect;

- initWithFrame:(NSRect)rect statusItem:(NSStatusItem*)item
{
    self = [super initWithFrame:rect];
    
    if (!self || !item)
        return nil;
    
    _statusItem = item;
    
    _isMenuExtra = [_statusItem respondsToSelector:@selector(drawMenuBackground:)]; 
    
    _smallFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0f];
    _bigFont = [NSFont fontWithName:@"Lucida Grande Bold" size:10.0f];
    
    _shadow = [[NSShadow alloc] init];
    
    [_shadow setShadowColor:[NSColor colorWithCalibratedWhite:1 alpha:0.55]];
    [_shadow setShadowOffset:CGSizeMake(0, -1.0)];
    [_shadow setShadowBlurRadius:1.0];
    
    return self;
}

- (void)drawRect:(NSRect)rect
{    
    if (!_engine || !_favorites) {
        
        NSImage *image = _isMenuDown ? _alternateImage : _image;
        
        if (image) 
            [image drawAtPoint:NSMakePoint(0, lround(([self frame].size.height - [image size].height) / 2)) fromRect:NSMakeRect(0, 0, [image size].width, [image size].height) operation:NSCompositeSourceOver fraction:1.0];
        
        return;
    }
        
    if (_isMenuExtra) {
        id menuExtra = (id)_statusItem;
        [menuExtra drawMenuBackground:YES];
    }
    else [_statusItem drawStatusBarBackgroundInRect:rect withHighlight:_isMenuDown];
    
    int offset = _isMenuExtra ? 0 : 3;
    
    if (_engine && [[_engine sensors] count] > 0) {
        
        int lastWidth = 0;
        int index = 0;
        
        for (int i = 0; i < [_favorites count]; i++) {
            
            id object = [_favorites objectAtIndex:i];
            
            if ([object isKindOfClass:[HWMonitorIcon class]]) {
                HWMonitorIcon *icon = (HWMonitorIcon*)object;
                
                if (icon) {
                    NSImage *image = _isMenuDown ? [icon alternateImage] : [icon image];
                    
                    if (image) {
                        
                        [image drawAtPoint:NSMakePoint(offset, lround(([self frame].size.height - [image size].height) / 2)) fromRect:NSMakeRect(0, 0, [image size].width, [image size].height) operation:NSCompositeSourceOver fraction:1.0];
                        
                        offset += [image size].width + 3;
                        
                        index = 0;
                    }
                }
            }
            else if ([object isKindOfClass:[HWMonitorSensor class]]) {
                HWMonitorSensor *sensor = (HWMonitorSensor*)object;
                
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
                
                [title addAttribute:NSForegroundColorAttributeName value:(_isMenuDown ? [NSColor whiteColor] : valueColor) range:NSMakeRange(0,[title length])];
                
                if (!_isMenuDown && useShadowEffect) 
                    [title addAttribute:NSShadowAttributeName value:_shadow range:NSMakeRange(0,[title length])];
                
                if (drawValuesInRow) {
                    [title addAttribute:NSFontAttributeName value:_bigFont range:NSMakeRange(0, [title length])];
                    
                    [title drawAtPoint:NSMakePoint(offset, ([self frame].size.height - [title size].height) / 2)];
                    
                    offset += [title size].width + 3;
                }
                else {
                    [title addAttribute:NSFontAttributeName value:_smallFont range:NSMakeRange(0, [title length])];
                    
                    int row = index % 2;
                    
                    [title drawAtPoint:NSMakePoint(offset, [_favorites count] == 1 ? ([self frame].size.height - [title size].height) / 2 + 1 : row == 0 ? [self frame].size.height / 2 - 1: [self frame].size.height / 2 - [title size].height + 2)];
                    
                    int width = [title size].width + 4;
                    
                    if (row == 1) {
                        width = width > lastWidth ? width : lastWidth;
                        offset += width;
                    }
                    else if ((i == [_favorites count] - 1) || ![[_favorites objectAtIndex:i+1] isKindOfClass:[HWMonitorSensor class]]) {
                        offset += width;
                    }
                    
                    lastWidth = width;
                }
                
                index++;
            }
        }
    }
    
    offset += _isMenuExtra ? -2 : 0;
    
    [self setFrameSize:NSMakeSize(offset, [self frame].size.height)];
    //snow leopard icon & text problem        
    [_statusItem setLength:([self frame].size.width)];
}

- (void)mouseDown:(NSEvent *)event
{
    if (!_isMenuExtra)
        [_statusItem popUpStatusItemMenu:[_statusItem menu]];
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
