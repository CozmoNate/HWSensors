//
//  HWMSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMSensor.h"
#import "HWMGroup.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"

#import "Localizer.h"

static NSDictionary *gHWMSensorLocalizationCache;

@implementation HWMSensor

@dynamic favorite;
@dynamic level;
@dynamic order;
@dynamic selector;
@dynamic type;
@dynamic value;

@dynamic group;

@synthesize engine;

-(NSString *)formattedValue
{
    if (self.value) {
        if (!gHWMSensorLocalizationCache) {
            gHWMSensorLocalizationCache = @{GetLocalizedString(@"%1.2fTHz") :@"%1.2fTHz",
                                            GetLocalizedString(@"%1.2fGHz") :@"%1.2fGHz",
                                            GetLocalizedString(@"%1.0fMHz") :@"%1.0fMHz",
                                            GetLocalizedString(@"%1.0frpm") :@"%1.0frpm",
                                            GetLocalizedString(@"%1.3fV")   :@"%1.3fV",
                                            GetLocalizedString(@"%1.2fA")   :@"%1.2fA",
                                            GetLocalizedString(@"%1.2fW")   :@"%1.2fW"};
        }

        switch (self.selector.unsignedIntegerValue) {
            case kHWMGroupTemperature:
            case kHWMGroupSmartTemperature:
                if (engine && engine.configuration.useFahrenheit) {
                    return [NSString stringWithFormat:@"%1.0f℉", self.value.floatValue * (9.0f / 5.0f) + 32.0f];
                }

                return [NSString stringWithFormat:@"%1.0f℃", self.value.floatValue];

            case kHWMGroupVoltage:
                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.3fV"], self.value.floatValue];

            case kHWMGroupPWM:
                return [NSString stringWithFormat:@"%1.0f%%", self.value.floatValue];

            case kHWMGroupTachometer:
                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.0frpm"], self.value.floatValue];

            case kHWMGroupMultiplier:
                return [NSString stringWithFormat:@"x%1.1f", self.value.floatValue];

            case kHWMGroupFrequency: {
                float floatValue = self.value.floatValue;

                if (floatValue > 1e6)
                    return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fTHz"], floatValue / 1e6];
                else if (floatValue > 1e3)
                    return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fGHz"], floatValue / 1e3];

                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.0fMHz"], floatValue];
            }

            case kHWMGroupCurrent:
                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fA"], self.value.floatValue];

            case kHWMGroupPower:
                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fW"], self.value.floatValue];

            case kHWMGroupBattery:
                return [NSString stringWithFormat:GetLocalizedString(@"%1.0f%%"), self.value.floatValue];

            case kHWMGroupSmartRemainingLife:
                return [NSString stringWithFormat:GetLocalizedString(@"%1.0f%%"), self.value.floatValue];
                
            case kHWMGroupSmartRemainingBlocks:
                return [NSString stringWithFormat:GetLocalizedString(@"%ld"), self.value.unsignedLongValue];
                
            default:
                break;
        }
    }

    return @"-";
}

@end
