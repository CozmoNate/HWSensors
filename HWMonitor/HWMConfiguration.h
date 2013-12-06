//
//  HWMConfiguration.h
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

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMColorTheme, HWMGraphsGroup, HWMItem, HWMSensorsGroup, HWMEngine;

@interface HWMConfiguration : NSManagedObject
{
    @private
    HWMColorTheme *_colorTheme;
}

@property (nonatomic, retain) NSNumber * graphsScaleValue;
@property (nonatomic, retain) NSNumber * graphsWindowAlwaysTopmost;
@property (nonatomic, retain) NSNumber * showVolumeNames;
@property (nonatomic, retain) NSNumber * smartSensorsUpdateRate;
@property (nonatomic, retain) NSNumber * smcSensorsUpdateRate;
@property (nonatomic, retain) NSNumber * updateSensorsInBackground;
@property (nonatomic, retain) NSNumber * useBigFontInMenubar;
@property (nonatomic, retain) NSNumber * useBsdDriveNames;
@property (nonatomic, retain) NSNumber * useFahrenheit;
@property (nonatomic, retain) NSNumber * useGraphSmoothing;
@property (nonatomic, retain) NSNumber * useShadowEffectsInMenubar;
@property (nonatomic, retain) NSNumber * colorThemeIndex;
@property (nonatomic, retain) NSOrderedSet *sensorGroups;
@property (nonatomic, retain) NSOrderedSet *graphGroups;
@property (nonatomic, retain) NSOrderedSet *colorThemes;
@property (nonatomic, retain) NSOrderedSet *favorites;

@property (readonly) HWMColorTheme *colorTheme;
//@property (nonatomic, strong) HWMEngine *engine;

@end

@interface HWMConfiguration (CoreDataGeneratedAccessors)

- (void)insertObject:(HWMSensorsGroup *)value inSensorGroupsAtIndex:(NSUInteger)idx;
- (void)removeObjectFromSensorGroupsAtIndex:(NSUInteger)idx;
- (void)insertSensorGroups:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeSensorGroupsAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInSensorGroupsAtIndex:(NSUInteger)idx withObject:(HWMSensorsGroup *)value;
- (void)replaceSensorGroupsAtIndexes:(NSIndexSet *)indexes withSensorGroups:(NSArray *)values;
- (void)addSensorGroupsObject:(HWMSensorsGroup *)value;
- (void)removeSensorGroupsObject:(HWMSensorsGroup *)value;
- (void)addSensorGroups:(NSOrderedSet *)values;
- (void)removeSensorGroups:(NSOrderedSet *)values;
- (void)insertObject:(HWMGraphsGroup *)value inGraphGroupsAtIndex:(NSUInteger)idx;
- (void)removeObjectFromGraphGroupsAtIndex:(NSUInteger)idx;
- (void)insertGraphGroups:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeGraphGroupsAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInGraphGroupsAtIndex:(NSUInteger)idx withObject:(HWMGraphsGroup *)value;
- (void)replaceGraphGroupsAtIndexes:(NSIndexSet *)indexes withGraphGroups:(NSArray *)values;
- (void)addGraphGroupsObject:(HWMGraphsGroup *)value;
- (void)removeGraphGroupsObject:(HWMGraphsGroup *)value;
- (void)addGraphGroups:(NSOrderedSet *)values;
- (void)removeGraphGroups:(NSOrderedSet *)values;
- (void)insertObject:(HWMColorTheme *)value inColorThemesAtIndex:(NSUInteger)idx;
- (void)removeObjectFromColorThemesAtIndex:(NSUInteger)idx;
- (void)insertColorThemes:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeColorThemesAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInColorThemesAtIndex:(NSUInteger)idx withObject:(HWMColorTheme *)value;
- (void)replaceColorThemesAtIndexes:(NSIndexSet *)indexes withColorThemes:(NSArray *)values;
- (void)addColorThemesObject:(HWMColorTheme *)value;
- (void)removeColorThemesObject:(HWMColorTheme *)value;
- (void)addColorThemes:(NSOrderedSet *)values;
- (void)removeColorThemes:(NSOrderedSet *)values;
- (void)insertObject:(HWMItem *)value inFavoritesAtIndex:(NSUInteger)idx;
- (void)removeObjectFromFavoritesAtIndex:(NSUInteger)idx;
- (void)insertFavorites:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeFavoritesAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInFavoritesAtIndex:(NSUInteger)idx withObject:(HWMItem *)value;
- (void)replaceFavoritesAtIndexes:(NSIndexSet *)indexes withFavorites:(NSArray *)values;
- (void)addFavoritesObject:(HWMItem *)value;
- (void)removeFavoritesObject:(HWMItem *)value;
- (void)addFavorites:(NSOrderedSet *)values;
- (void)removeFavorites:(NSOrderedSet *)values;
@end
