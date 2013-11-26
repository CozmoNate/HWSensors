//
//  HWMSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMSensor.h"
#import "HWMSensorsGroup.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"

#import "Localizer.h"

static NSDictionary *gHWMSensorLocalizationCache;

@implementation HWMSensor

@dynamic level;
@dynamic service;
@dynamic selector;
@dynamic type;
@dynamic value;

@dynamic group;

-(void)awakeFromFetch
{
    [self setPrimitiveValue:@0 forKey:@"service"];
}

-(BOOL)isActive
{
    return self.service && self.service.unsignedLongLongValue;
}

-(NSString *)formattedValue
{
    if (self.value) {
        if (!gHWMSensorLocalizationCache) {
            gHWMSensorLocalizationCache = @{    @"%1.2fTHz" : GetLocalizedString(@"%1.2fTHz"),
                                                @"%1.2fGHz" : GetLocalizedString(@"%1.2fGHz"),
                                                @"%1.0fMHz" : GetLocalizedString(@"%1.0fMHz"),
                                                @"%1.0frpm" : GetLocalizedString(@"%1.0frpm"),
                                                @"%1.3fV" : GetLocalizedString(@"%1.3fV"),
                                                @"%1.2fA" : GetLocalizedString(@"%1.2fA"),
                                                @"%1.2fW" : GetLocalizedString(@"%1.2fW")};
        }

        float floatValue = self.value.floatValue;

        switch (self.selector.unsignedIntegerValue) {
            case kHWMGroupTemperature:
            case kHWMGroupSmartTemperature:
                if (self.engine.configuration.useFahrenheit.boolValue) {
                    return [NSString stringWithFormat:@"%1.0f℉", floatValue * (9.0f / 5.0f) + 32.0f];
                }

                return [NSString stringWithFormat:@"%1.0f℃", floatValue];

            case kHWMGroupVoltage:
                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.3fV"], floatValue];

            case kHWMGroupPWM:
                return [NSString stringWithFormat:@"%1.0f%%", floatValue];

            case kHWMGroupTachometer:
                if (floatValue) {
                    return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.0frpm"], floatValue];
                }
                break;

            case kHWMGroupMultiplier:
                return [NSString stringWithFormat:@"x%1.1f", floatValue];

            case kHWMGroupFrequency:
                if (floatValue > 1e6)
                    return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fTHz"], floatValue / 1e6];
                else if (floatValue > 1e3)
                    return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fGHz"], floatValue / 1e3];

                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.0fMHz"], floatValue];

            case kHWMGroupCurrent:
                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fA"], floatValue];

            case kHWMGroupPower:
                return [NSString stringWithFormat:[gHWMSensorLocalizationCache objectForKey:@"%1.2fW"], floatValue];

            case kHWMGroupBattery:
                return [NSString stringWithFormat:GetLocalizedString(@"%1.0f%%"), floatValue];

            case kHWMGroupSmartRemainingLife:
                return [NSString stringWithFormat:GetLocalizedString(@"%1.0f%%"), 100 - floatValue];
                
            case kHWMGroupSmartRemainingBlocks:
                return [NSString stringWithFormat:GetLocalizedString(@"%ld"), self.value.unsignedLongValue];
                
            default:
                break;
        }
    }

    return @"-";
}

- (void)doUpdateValue
{
    //
}

@end
