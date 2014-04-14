//
//  HWMConfiguration.m
//  HWMonitor
//
//  Created by Kozlek on 07.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMFavorite.h"
#import "HWMGraphsGroup.h"
#import "HWMSensorsGroup.h"

#import "SmcHelper.h"
#import "FakeSMCDefinitions.h"

@implementation HWMConfiguration

@dynamic graphsScaleValue;
@dynamic graphsWindowAlwaysTopmost;
@dynamic notifyAlarmLevelChanges;
@dynamic showSensorLegendsInPopup;
@dynamic smartSensorsUpdateRate;
@dynamic smcSensorsUpdateRate;
@dynamic updateSensorsInBackground;
@dynamic useBigFontInMenubar;
@dynamic useFahrenheit;
@dynamic useGraphSmoothing;
@dynamic useShadowEffectsInMenubar;
@dynamic driveLegendSelector;
@dynamic driveNameSelector;
@dynamic colorThemeIndex;
@dynamic colorThemes;
@dynamic favorites;
@dynamic graphGroups;
@dynamic sensorGroups;

@synthesize colorTheme = _colorTheme;

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

-(void)setColorTheme:(HWMColorTheme *)colorTheme
{
    NSUInteger index = [self.colorThemes indexOfObject:colorTheme];
    
    if (index != NSNotFound) {
        [self setColorThemeIndex:[NSNumber numberWithUnsignedInteger:index]];
    }
}

-(HWMColorTheme *)colorTheme
{
    if (!_colorTheme) {
        _colorTheme = [self.colorThemes objectAtIndex:self.colorThemeIndex.unsignedIntegerValue];
    }
    
    return _colorTheme;
}

//-(void)addSensorGroupsObject:(HWMSensorsGroup *)value
//{
//    [[self mutableOrderedSetValueForKey:@"sensorGroups"] addObject:value];
//}
//
//-(void)addGraphGroupsObject:(HWMGraphsGroup *)value
//{
//    [[self mutableOrderedSetValueForKey:@"graphGroups"] addObject:value];
//}
//
//-(void)addColorThemesObject:(HWMColorTheme *)value
//{
//    [[self mutableOrderedSetValueForKey:@"colorThemes"] addObject:value];
//}

//-(void)addFavoritesObject:(HWMFavorite *)value
//{
//    NSMutableOrderedSet * favorites = [self mutableOrderedSetValueForKey:@"favorites"];
//    
//    [favorites addObject:value];
//}
//
//- (void)insertObject:(HWMFavorite *)value inFavoritesAtIndex:(NSUInteger)idx
//{
//    NSMutableOrderedSet * favorites = [self mutableOrderedSetValueForKey:@"favorites"];
//    
//    [favorites insertObject:value atIndex:idx];
//}
//
//-(void)removeFavoritesAtIndexes:(NSIndexSet *)indexes
//{
//    NSMutableOrderedSet * favorites = [self mutableOrderedSetValueForKey:@"favorites"];
//    
//    [favorites removeObjectsAtIndexes:indexes];
//}

@end
