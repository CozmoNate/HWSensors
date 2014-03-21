//
//  PopupBasicCell.m
//  HWMonitor
//
//  Created by Kozlek on 22/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
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

#import "PopupBasicCell.h"
#import "HWMEngine.h"
#import "HWMItem.h"
#import "HWMConfiguration.h"

@implementation PopupBasicCell

@synthesize colorTheme = _colorTheme;

-(id)init
{
    self = [super init];

    if (self) {
        [self initialize];
    }

    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self) {
        [self initialize];
    }

    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

    if (self) {
        [self initialize];
    }

    return self;
}

-(void)dealloc
{
    [self deallocate];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"objectValue.engine.configuration.colorTheme"]) {
        _colorTheme = [(HWMItem*)self.objectValue engine].configuration.colorTheme;
        [self colorThemeChanged:_colorTheme];
    }
}

- (void)colorThemeChanged:(HWMColorTheme*)newColorTheme
{

}
- (void)initialize
{
    [self addObserver:self forKeyPath:@"objectValue.engine.configuration.colorTheme" options:NSKeyValueObservingOptionNew context:nil];
}

- (void)deallocate
{
    [self removeObserver:self forKeyPath:@"objectValue.engine.configuration.colorTheme"];
}

@end
