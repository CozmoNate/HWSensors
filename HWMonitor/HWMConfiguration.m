//
//  HWMConfiguration.m
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMGraphsGroup.h"
#import "HWMItem.h"
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

@synthesize engine;
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
