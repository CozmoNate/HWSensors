//
//  HWMonitorView.m
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

// Based on code by:
//
//  DeVercruesseASMView.m
//
//  Created by Frank Vercruesse on Sat Aug 25 2001.
//  Copyright (c) 2001 Frank Vercruesse.
//
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#import "HWMonitorView.h"
#import "HWMonitorIcon.h"

@implementation HWMonitorView

@synthesize engine=_engine;
@synthesize favorites=_favorites;

@synthesize useBigFont;
@synthesize useShadowEffect;

-(id)initWithFrame:(NSRect)rect menuExtra:(NSMenuExtra *)statusItem
{
    self = [super initWithFrame:rect menuExtra:statusItem];
    
    if (!self || !statusItem)
        return nil;
    
    _smallFont = [NSFont boldSystemFontOfSize:9.0f];
    _bigFont = [NSFont boldSystemFontOfSize:11.0f];
    
    _shadow = [[NSShadow alloc] init];
    
    [_shadow setShadowColor:[NSColor colorWithCalibratedWhite:1 alpha:0.55]];
    [_shadow setShadowOffset:CGSizeMake(0, -1.0)];
    [_shadow setShadowBlurRadius:1.0];
    
    return self;
}

- (void)drawRect:(NSRect)rect
{    
    [_menuExtra drawMenuBackground:[_menuExtra isMenuDown]];
    
    if (!_engine || !_favorites || [_favorites count] == 0) {
        
        NSImage *image = [_menuExtra isMenuDown] ? _alternateImage : _image;
        
        if (image) {
            NSUInteger width = [image size].width > 22 ? [image size].width : 22;
            
            [image drawAtPoint:NSMakePoint(lround((width - [image size].width) / 2), lround(([self frame].size.height - [image size].height) / 2)) fromRect:NSMakeRect(0, 0, width, [image size].height) operation:NSCompositeSourceOver fraction:1.0];
            
            [self setFrameSize:NSMakeSize(width, [self frame].size.height)];
        }
        //snow leopard icon & text problem        
        [_menuExtra setLength:([self frame].size.width)];
        
        return;
    }
    
    int offset = 0;
    
    if (_engine && [[_engine sensors] count] > 0) {
        
        int lastWidth = 0;
        int index = 0;
        
        for (int i = 0; i < [_favorites count]; i++) {
            
            id object = [_favorites objectAtIndex:i];
            
            if ([object isKindOfClass:[HWMonitorIcon class]]) {
                HWMonitorIcon *icon = (HWMonitorIcon*)object;
                
                if (icon) {
                    NSImage *image = [_menuExtra isMenuDown] ? [icon alternateImage] : [icon image];
                    
                    if (image) {
                        
                        [image compositeToPoint:NSMakePoint(offset, lround(([self frame].size.height - [image size].height) / 2)) fromRect:NSMakeRect(0, 0, [image size].width, [image size].height) operation:NSCompositeSourceOver fraction:1.0];
                        
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
                
                [title addAttribute:NSForegroundColorAttributeName value:([_menuExtra isMenuDown] ? [NSColor whiteColor] : valueColor) range:NSMakeRange(0,[title length])];
                
                if (![_menuExtra isMenuDown] && useShadowEffect) 
                    [title addAttribute:NSShadowAttributeName value:_shadow range:NSMakeRange(0,[title length])];
                
                if (useBigFont) {
                    [title addAttribute:NSFontAttributeName value:_bigFont range:NSMakeRange(0, [title length])];
                    
                    [title drawAtPoint:NSMakePoint(offset, lround(([self frame].size.height - [title size].height) / 2))];
                    
                    offset += [title size].width + 3;
                }
                else {
                    [title addAttribute:NSFontAttributeName value:_smallFont range:NSMakeRange(0, [title length])];
                    
                    int row = index % 2;

                    [title drawAtPoint:NSMakePoint(offset, [_favorites count] == 1 ? lround(([self frame].size.height - [title size].height) / 2) + 1 : row == 0 ? lround([self frame].size.height / 2) - 1: lround([self frame].size.height / 2) - [title size].height + 2)];
                    
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
    
    [self setFrameSize:NSMakeSize(offset, [self frame].size.height)];
    
    //snow leopard icon & text problem        
    [_menuExtra setLength:([self frame].size.width)];
}

@end
