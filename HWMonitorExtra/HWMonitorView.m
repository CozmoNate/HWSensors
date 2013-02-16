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

const NSString* kHWMonitorViewSpacer = @" ";

@implementation HWMonitorView

@synthesize engine = _engine;
@synthesize favorites = _favorites;

@synthesize useBigFont = _useBigFont;
@synthesize useShadowEffect = _useShadowEffect;

-(id)initWithFrame:(NSRect)rect menuExtra:(NSMenuExtra *)statusItem
{
    self = [super initWithFrame:rect menuExtra:statusItem];
    
    if (!self || !statusItem)
        return nil;
    
    _smallFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0];
    _bigFont = [NSFont fontWithName:@"Lucida Grande" size:13.9];
    
    _shadow = [[NSShadow alloc] init];
    
    [_shadow setShadowColor:[NSColor colorWithCalibratedWhite:1 alpha:0.55]];
    [_shadow setShadowOffset:CGSizeMake(0, -1.0)];
    [_shadow setShadowBlurRadius:1.0];
    
    [_menuExtra setView:self];
    
    return self;
}

- (void)drawRect:(NSRect)rect
{   
    if ([_menuExtra isMenuDown])
        [_menuExtra drawMenuBackground:YES];
    
    NSGraphicsContext* context = [NSGraphicsContext currentContext];
    
    if (!_engine || !_favorites || [_favorites count] == 0) {
        
        [context saveGraphicsState];
        
        if (![_menuExtra isMenuDown])
            [_shadow set];
        
        NSImage *image = [_menuExtra isMenuDown] ? _alternateImage : _image;
        
        if (image) {
            NSUInteger width = [image size].width + 4;
            
            [image drawAtPoint:NSMakePoint(lround((width - [image size].width) / 2), lround(([self frame].size.height - [image size].height) / 2)) fromRect:NSMakeRect(0, 0, width, [image size].height) operation:NSCompositeSourceOver fraction:1.0];
            
            [self setFrameSize:NSMakeSize(width, [self frame].size.height)];
        }
        
        [context restoreGraphicsState];
        
        //snow leopard icon & text problem
        [_menuExtra setLength:([self frame].size.width)];
        
        return;
    }
    
    int offset = 0;
    
    NSAttributedString *spacer = [[NSAttributedString alloc] initWithString:_useBigFont ? @" " : @"  " attributes:[NSDictionary dictionaryWithObjectsAndKeys:_useBigFont ? _bigFont : _smallFont, NSFontAttributeName, nil]];
    
    if (_engine && [[_engine sensors] count] > 0) {
        
        int lastWidth = 0;
        int index = 0;
        
        for (NSUInteger i = 0; i < [_favorites count]; i++) {
            
            id object = [_favorites objectAtIndex:i];
            
            if ([object isKindOfClass:[HWMonitorIcon class]]) {
                [context saveGraphicsState];
                
                if (![_menuExtra isMenuDown])
                    [_shadow set];
                
                HWMonitorIcon *icon = (HWMonitorIcon*)object;
                
                if (icon) {
                    NSImage *image = [_menuExtra isMenuDown] ? [icon alternateImage] : [icon image];
                    
                    if (image) {
                        
                        [image drawAtPoint:NSMakePoint(offset, lround(([self frame].size.height - [image size].height) / 2)) fromRect:NSMakeRect(0, 0, [image size].width, [image size].height) operation:NSCompositeSourceOver fraction:1.0];
                        
                        offset = offset + [image size].width + (i + 1 < [_favorites count] && [[_favorites objectAtIndex:i + 1] isKindOfClass:[HWMonitorSensor class]] ? 2 : i + 1 == [_favorites count] ? 0 : [spacer size].width);
                        
                        index = 0;
                    }
                }
                
                [context restoreGraphicsState];
            }
            else if ([object isKindOfClass:[HWMonitorSensor class]]) {
                [context saveGraphicsState];

                HWMonitorSensor *sensor = (HWMonitorSensor*)object;
                
                if (!sensor) 
                    continue;
                
                NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:[sensor formattedValue]];
                
                NSColor *valueColor;
                
                switch ([sensor level]) {
                        /*case kHWSensorLevelDisabled:
                         break;
                         
                         case kHWSensorLevelNormal:
                         break;*/
                        
                    case kHWSensorLevelModerate:
                        valueColor = [NSColor colorWithCalibratedRed:0.7f green:0.3f blue:0.03f alpha:1.0f];
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
                
                if (![_menuExtra isMenuDown] && _useShadowEffect)
                    [_shadow set];
                
                if (_useBigFont) {
                    
                    CGContextRef cgContext = [context graphicsPort];
                    CGContextSetShouldSmoothFonts(cgContext, NO);
                    
                    [title addAttribute:NSFontAttributeName value:_bigFont range:NSMakeRange(0, [title length])];
                    [title drawAtPoint:NSMakePoint(offset, lround(([self frame].size.height - [title size].height) / 2))];
                    
                    offset = offset + [title size].width + (i + 1 < [_favorites count] ? [spacer size].width : 0);
                }
                else {
                    int row = index % 2;
                    
                    [title addAttribute:NSFontAttributeName value:_smallFont range:NSMakeRange(0, [title length])];
                    [title drawAtPoint:NSMakePoint(offset, [_favorites count] == 1 ? lround(([self frame].size.height - [title size].height) / 2) + 1 : row == 0 ? lround([self frame].size.height / 2) - 1 : lround([self frame].size.height / 2) - [title size].height + 2)];
                    
                    int width = [title size].width;
                    
                    if (row == 0) {
                        if (i + 1 == [_favorites count]) {
                            offset += width;
                        }
                        else if (i + 1 < [_favorites count] && ![[_favorites objectAtIndex:i + 1] isKindOfClass:[HWMonitorSensor class]]) {
                            offset = offset + width + [spacer size].width;
                        }
                    }
                    else if (row == 1) {
                        width = width > lastWidth ? width : lastWidth;
                        offset = offset + width + (i + 1 < [_favorites count] ? [spacer size].width : 0);
                    }
                    
                    lastWidth = width;
                }
                
                index++;
                
                [context restoreGraphicsState];
            }
        }
    }
    
    [self setFrameSize:NSMakeSize(offset, [self frame].size.height)];
    
    //snow leopard icon & text problem        
    [_menuExtra setLength:([self frame].size.width)];
}

@end
