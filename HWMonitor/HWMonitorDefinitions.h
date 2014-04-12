//
//  HWMonitorDefinitions.h
//  HWSensors
//
//  Created by kozlek on 26.05.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#ifndef HWSensors_HWMonitorDefinitions_h
#define HWSensors_HWMonitorDefinitions_h

#import "EXTKeyPathCoding.h"

// Icons
#define kHWMonitorIconHWMonitor             @"hwmonitor"
#define kHWMonitorIconScale                 @"scale"
#define kHWMonitorIconThermometer           @"thermometer"
#define kHWMonitorIconDevice                @"pcicard"
#define kHWMonitorIconTemperatures          @"temperatures"
#define kHWMonitorIconHddTemperatures       @"hddtemperatures"
#define kHWMonitorIconSsdLife               @"ssdlife"
#define kHWMonitorIconMultipliers           @"multipliers"
#define kHWMonitorIconFrequencies           @"frequencies"
#define kHWMonitorIconTachometers           @"tachometers"
#define kHWMonitorIconVoltages              @"voltages"
#define kHWMonitorIconBattery               @"battery"

// Misc
#define kHWMonitorPrefsItemDataType         @"kHWMonitorPrefsItemDataType"
#define kHWMonitorPopupItemDataType         @"kHWMonitorPopupItemDataType"
#define kHWMonitorGraphsItemDataType        @"kHWMonitorGraphsItemDataType"


#define NotifierSensorLevelExceededNotification             @"Alarm Level Exceeded"
#define NotifierSensorLevelHighNotification                 @"Alarm High Level"
#define NotifierSensorLevelModerateNotification             @"Alarm Moderate Level"

#define NotifierSensorLevelExceededHumanReadableDescription	@"Sensor alarm level exceeded limit"
#define NotifierSensorLevelHighHumanReadableDescription		@"Sensor high alarm level"
#define NotifierSensorLevelModerateHumanReadableDescription	@"Sensor moderate alarm level"


// TEST BATTERY
//#define kHWMonitorDebugBattery

#endif
