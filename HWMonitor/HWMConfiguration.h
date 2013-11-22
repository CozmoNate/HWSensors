//
//  HWMConfiguration.h
//  HWMonitor
//
//  Created by Kozlek on 21/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMColorTheme;

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
@property (nonatomic, retain) HWMColorTheme *colorTheme;

@end
