//
//  HWMonitorDefinitions.h
//  HWSensors
//
//  Created by kozlek on 26.05.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#ifndef HWSensors_HWMonitorDefinitions_h
#define HWSensors_HWMonitorDefinitions_h

#define kHWMonitorMenuTitleWidth            22 // '9' char width multiplied by this number
#define kHWMonitorMenuValueWidth            8  // '9' char width multiplied by this number

// Preferences
#define kHWMonitorFavoritesList             @"favoritesList"
#define kHWMonitorHiddenList                @"hiddenList"
#define kHWMonitorUseFahrenheitKey          @"useFahrenheit"
#define kHWMonitorUseBigStatusMenuFont      @"useBigFont"
#define kHWMonitorUseShadowEffect           @"useShadowEffect"
#define kHWMonitorUseBSDNames               @"showBSDNames"
#define kHWMonitorShowVolumeNames           @"showVolumeNames"
#define kHWMonitorSelectedTag               @"selectedTag"
#define kHWMonitorSmcSensorsUpdateRate      @"smcSensorsUpdateRate"
#define kHWMonitorSmartSensorsUpdateRate    @"smartSensorsUpdateRate"

// Icons
#define kHWMonitorIconDefault               @"hwmonitor"
#define kHWMonitorIconThermometer           @"thermometer"
#define kHWMonitorIconDevice                @"pcicard"
#define kHWMonitorIconTemperatures          @"temperatures"
#define kHWMonitorIconHddTemperatures       @"hddtemperatures"
#define kHWMonitorIconSsdLife               @"ssdlife"
#define kHWMonitorIconMultipliers           @"multipliers"
#define kHWMonitorIconFrequencies           @"frequencies"
#define kHWMonitorIconTachometers           @"tachometers"
#define kHWMonitorIconVoltages              @"voltages"

// Notifications
#define HWMonitorBooleanYES                 @"YES"
#define HWMonitorBooleanNO                  @"NO"
#define HWMonitorFavoritesChanged           @"HWMonitorFavoritesChanged"
#define HWMonitorUseFahrenheitChanged       @"HWMonitorUseFahrenheitChanged"
#define HWMonitorUseBigFontChanged          @"HWMonitorUseBigFontChanged"
#define HWMonitorUseShadowsChanged          @"HWMonitorUseShadowsChanged"
#define HWMonitorShowHiddenChanged          @"HWMonitorShowHiddenChanged"
#define HWMonitorUseBSDNamesChanged         @"HWMonitorUseBSDNamesChanged"
#define HWMonitorShowVolumeNamesChanged     @"HWMonitorShowVolumeNamesChanged"
#define HWMonitorRequestItems               @"HWMonitorRequestItems"
#define HWMonitorRecieveItems               @"HWMonitorRecieveItems"
#define HWMonitorValuesChanged              @"HWMonitorValuesChanged"
#define HWMonitorAppIsActive                @"HWMonitorAppIsActive"
#define HWMonitorUpdateRateChanged          @"HWMonitorUpdateRateChanged"

// Key
#define kHWMonitorKeyName                   @"Name"
#define kHWMonitorKeyIcon                   @"Icon"
#define kHWMonitorKeyValue                  @"Value"
#define kHWMonitorKeyRawValue               @"RawValue"
#define kHWMonitorKeySeparator              @"IsSeparator"
#define kHWMonitorKeyVisible                @"Visible"
#define kHWMonitorKeyFavorite               @"Favorite"
#define kHWMonitorKeyKey                    @"Key"
#define kHWMonitorKeyGroup                  @"Group"
#define kHWMonitorKeyTitle                  @"Title"
#define kHWMonitorKeyIndex                  @"Index"
#define kHWMonitorKeyEnabled                @"Enabled"
#define kHWMonitorKeyColor                  @"Color"
#define kHWMonitorKeyPoints                 @"Points"

// Misc
#define kHWMonitorTableViewDataType         @"kHWMonitorTableViewDataType"

#endif
