//
//  HWMConfiguration.h
//  HWMonitor
//
//  Created by Kozlek on 07.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMColorTheme, HWMFavorite, HWMGraphsGroup, HWMSensorsGroup;

@interface HWMConfiguration : NSManagedObject

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
@property (nonatomic, retain) NSOrderedSet *colorThemes;
@property (nonatomic, retain) NSOrderedSet *favorites;
@property (nonatomic, retain) NSOrderedSet *graphGroups;
@property (nonatomic, retain) NSOrderedSet *sensorGroups;
@property (nonatomic, retain) HWMColorTheme *colorTheme;
@end

@interface HWMConfiguration (CoreDataGeneratedAccessors)

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
- (void)insertObject:(HWMFavorite *)value inFavoritesAtIndex:(NSUInteger)idx;
- (void)removeObjectFromFavoritesAtIndex:(NSUInteger)idx;
- (void)insertFavorites:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeFavoritesAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInFavoritesAtIndex:(NSUInteger)idx withObject:(HWMFavorite *)value;
- (void)replaceFavoritesAtIndexes:(NSIndexSet *)indexes withFavorites:(NSArray *)values;
- (void)addFavoritesObject:(HWMFavorite *)value;
- (void)removeFavoritesObject:(HWMFavorite *)value;
- (void)addFavorites:(NSOrderedSet *)values;
- (void)removeFavorites:(NSOrderedSet *)values;
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
@end
