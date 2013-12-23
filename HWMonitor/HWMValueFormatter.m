//
//  HWMValueFormatter.m
//  HWMonitor
//
//  Created by Kozlek on 16.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMValueFormatter.h"
#import "Localizer.h"
#import "HWMSensorsGroup.h"
#import "HWMConfiguration.h"

static NSDictionary *gHWMValueFormatterFullFormatLocalizationCache;
static NSDictionary *gHWMValueFormatterStrippedFormatLocalizationCache;

@implementation HWMValueFormatter

+ (NSString*)formattedValue:(NSNumber*)value usingRulesOfGroup:(NSNumber*)selector configuration:(HWMConfiguration*)configuration
{
    if (value) {
        if (!gHWMValueFormatterFullFormatLocalizationCache) {
            gHWMValueFormatterFullFormatLocalizationCache = @{@"%1.2f THz"  : GetLocalizedString(@"%1.2f THz"),
                                                              @"%1.2f GHz"  : GetLocalizedString(@"%1.2f GHz"),
                                                              @"%1.0f MHz"  : GetLocalizedString(@"%1.0f MHz"),
                                                              @"%1.0f rpm"  : GetLocalizedString(@"%1.0f rpm"),
                                                              @"%1.3f V"    : GetLocalizedString(@"%1.3f V"),
                                                              @"%1.2f A"    : GetLocalizedString(@"%1.2f A"),
                                                              @"%1.2f W"    : GetLocalizedString(@"%1.2f W")};
        }
        
        float floatValue = value.floatValue;
        
        switch (selector.unsignedIntegerValue) {
            case kHWMGroupTemperature:
            case kHWMGroupSmartTemperature:
                if (configuration.useFahrenheit.boolValue) {
                    return [NSString stringWithFormat:@"%1.0f℉", floatValue * (9.0f / 5.0f) + 32.0f];
                }
                
                return [NSString stringWithFormat:@"%1.0f℃", floatValue];
                
            case kHWMGroupVoltage:
                return [NSString stringWithFormat:[gHWMValueFormatterFullFormatLocalizationCache objectForKey:@"%1.3f V"], floatValue];
                
            case kHWMGroupTachometer:
                if (floatValue) {
                    return [NSString stringWithFormat:[gHWMValueFormatterFullFormatLocalizationCache objectForKey:@"%1.0f rpm"], floatValue];
                }
                break;
                
            case kHWMGroupMultiplier:
                return [NSString stringWithFormat:@"x%1.1f", floatValue];
                
            case kHWMGroupFrequency:
                if (floatValue > 1e6)
                    return [NSString stringWithFormat:[gHWMValueFormatterFullFormatLocalizationCache objectForKey:@"%1.2f THz"], floatValue / 1e6];
                else if (floatValue > 1e3)
                    return [NSString stringWithFormat:[gHWMValueFormatterFullFormatLocalizationCache objectForKey:@"%1.2f GHz"], floatValue / 1e3];
                
                return [NSString stringWithFormat:[gHWMValueFormatterFullFormatLocalizationCache objectForKey:@"%1.0f MHz"], floatValue];
                
            case kHWMGroupCurrent:
                return [NSString stringWithFormat:[gHWMValueFormatterFullFormatLocalizationCache objectForKey:@"%1.2f A"], floatValue];
                
            case kHWMGroupPower:
                return [NSString stringWithFormat:[gHWMValueFormatterFullFormatLocalizationCache objectForKey:@"%1.2f W"], floatValue];
                
            case kHWMGroupPWM:
            case kHWMGroupBattery:
            case kHWMGroupBatteryInternal:
            case kHWMGroupBatteryMouse:
            case kHWMGroupBatteryKeyboard:
            case kHWMGroupBatteryTrackpad:
            case kHWMGroupSmartRemainingLife:
                return [NSString stringWithFormat:@"%1.0f%%", floatValue];
                
            case kHWMGroupSmartRemainingBlocks:
                return [NSString stringWithFormat:@"%ld", value.unsignedLongValue];
                
            default:
                break;
        }
    }
    
    return @"-";
}

+ (NSString*)strippedValue:(NSNumber*)value usingRulesOfGroup:(NSNumber*)selector configuration:(HWMConfiguration*)configuration {
    
    if (value) {
        if (!gHWMValueFormatterStrippedFormatLocalizationCache) {
            gHWMValueFormatterStrippedFormatLocalizationCache = @{@"%1.2fTHz"  : GetLocalizedString(@"%1.2fTHz"),
                                                                  @"%1.2fGHz"  : GetLocalizedString(@"%1.2fGHz"),
                                                                  @"%1.0fMHz"  : GetLocalizedString(@"%1.0fMHz"),
                                                                  @"%1.0frpm"  : GetLocalizedString(@"%1.0frpm"),
                                                                  @"%1.3fV"    : GetLocalizedString(@"%1.3fV"),
                                                                  @"%1.2fA"    : GetLocalizedString(@"%1.2fA"),
                                                                  @"%1.2fW"    : GetLocalizedString(@"%1.2fW")};
        }
        
        float floatValue = value.floatValue;
        
        switch (selector.unsignedIntegerValue) {
            case kHWMGroupTemperature:
            case kHWMGroupSmartTemperature:
                if (configuration.useFahrenheit.boolValue) {
                    return [NSString stringWithFormat:@"%1.0f°", floatValue * (9.0f / 5.0f) + 32.0f];
                }
                
                return [NSString stringWithFormat:@"%1.0f°", floatValue];
                
            case kHWMGroupVoltage:
                return [NSString stringWithFormat:[gHWMValueFormatterStrippedFormatLocalizationCache objectForKey:@"%1.3fV"], floatValue];
                
            case kHWMGroupTachometer:
                if (floatValue) {
                    return [NSString stringWithFormat:[gHWMValueFormatterStrippedFormatLocalizationCache objectForKey:@"%1.0frpm"], floatValue];
                }
                break;
                
            case kHWMGroupMultiplier:
                return [NSString stringWithFormat:@"x%1.1f", floatValue];
                
            case kHWMGroupFrequency:
                if (floatValue > 1e6)
                    return [NSString stringWithFormat:[gHWMValueFormatterStrippedFormatLocalizationCache objectForKey:@"%1.2fTHz"], floatValue / 1e6];
                else if (floatValue > 1e3)
                    return [NSString stringWithFormat:[gHWMValueFormatterStrippedFormatLocalizationCache objectForKey:@"%1.2fGHz"], floatValue / 1e3];
                
                return [NSString stringWithFormat:[gHWMValueFormatterStrippedFormatLocalizationCache objectForKey:@"%1.0fMHz"], floatValue];
                
            case kHWMGroupCurrent:
                return [NSString stringWithFormat:[gHWMValueFormatterStrippedFormatLocalizationCache objectForKey:@"%1.2fA"], floatValue];
                
            case kHWMGroupPower:
                return [NSString stringWithFormat:[gHWMValueFormatterStrippedFormatLocalizationCache objectForKey:@"%1.2fW"], floatValue];
                
            case kHWMGroupPWM:
            case kHWMGroupBattery:
            case kHWMGroupBatteryInternal:
            case kHWMGroupBatteryMouse:
            case kHWMGroupBatteryKeyboard:
            case kHWMGroupBatteryTrackpad:
                return [NSString stringWithFormat:@"%1.0f%%", floatValue];
                
            case kHWMGroupSmartRemainingLife:
                return [NSString stringWithFormat:@"%1.0f%%", 100 - floatValue];
                
            case kHWMGroupSmartRemainingBlocks:
                return [NSString stringWithFormat:@"%ld", value.unsignedLongValue];
                
            default:
                break;
        }
    }
    
    return @"-";
}

@end
