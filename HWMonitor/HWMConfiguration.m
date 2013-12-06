//
//  HWMConfiguration.m
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
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

#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMGraphsGroup.h"
#import "HWMItem.h"
#import "HWMIcon.h"
#import "HWMSensorsGroup.h"


@implementation HWMConfiguration

@dynamic graphsScaleValue;
@dynamic graphsWindowAlwaysTopmost;
@dynamic showVolumeNames;
@dynamic smartSensorsUpdateRate;
@dynamic smcSensorsUpdateRate;
@dynamic updateSensorsInBackground;
@dynamic useBigFontInMenubar;
@dynamic useBsdDriveNames;
@dynamic useFahrenheit;
@dynamic useGraphSmoothing;
@dynamic useShadowEffectsInMenubar;
@dynamic colorThemeIndex;
@dynamic sensorGroups;
@dynamic graphGroups;
@dynamic colorThemes;
@dynamic favorites;

//@synthesize engine;
@synthesize colorTheme;

-(void)setColorThemeIndex:(NSNumber *)colorThemeIndex
{
    if (![self.colorThemeIndex isEqualToNumber:colorThemeIndex]) {
        [self willChangeValueForKey:@"colorThemeIndex"];
        [self setPrimitiveValue:colorThemeIndex forKey:@"colorThemeIndex"];
        [self didChangeValueForKey:@"colorThemeIndex"];

        [self willChangeValueForKey:@"colorTheme"];
        _colorTheme = nil;
        [self didChangeValueForKey:@"colorTheme"];
    }
}

-(HWMColorTheme *)colorTheme
{
    if (!_colorTheme) {
        _colorTheme = [self.colorThemes objectAtIndex:self.colorThemeIndex.unsignedIntegerValue];
    }

    return _colorTheme;
}

-(void)addSensorGroupsObject:(HWMSensorsGroup *)value
{
    [[self mutableOrderedSetValueForKey:@"sensorGroups"] addObject:value];
}

-(void)addGraphGroupsObject:(HWMGraphsGroup *)value
{
    [[self mutableOrderedSetValueForKey:@"graphGroups"] addObject:value];
}

-(void)addColorThemesObject:(HWMColorTheme *)value
{
    [[self mutableOrderedSetValueForKey:@"colorThemes"] addObject:value];
}

-(void)addFavoritesObject:(HWMColorTheme *)value
{
    [[self mutableOrderedSetValueForKey:@"favorites"] addObject:value];
}

@end
