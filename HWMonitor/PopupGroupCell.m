//
//  HWMonitorGroupCell.m
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import "PopupGroupCell.h"
#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensorsGroup.h"

@implementation PopupGroupCell

- (void)drawRect:(NSRect)dirtyRect
{
    if (!_gradient) {
        _gradient = [[NSGradient alloc] initWithStartingColor:self.colorTheme.groupStartColor
                                                  endingColor:self.colorTheme.groupEndColor];
    }
    
    NSRect contentRect = [self bounds];
    
    [_gradient drawInRect:NSMakeRect(contentRect.origin.x + 1.0, contentRect.origin.y, contentRect.size.width - 2.0, contentRect.size.height) angle:270];
}

-(void)colorThemeChanged:(HWMColorTheme *)newColorTheme
{
    _gradient = nil;
    [self setNeedsDisplay:YES];
}

@end
