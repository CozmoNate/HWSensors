//
//  HWMAtaSmartSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
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

#import "HWMAtaSmartSensor.h"

#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensorsGroup.h"

#import "HWMonitorDefinitions.h"
#import "Localizer.h"

#import <sys/mount.h>
#import <Growl/Growl.h>

#import "NSString+returnCodeDescription.h"

#pragma mark
#pragma mark HWMATASmartInterfaceWrapper

#define RAW_TO_LONG(attribute)  (UInt64)attribute->rawvalue[0] | \
(UInt64)attribute->rawvalue[1] << 8 | \
(UInt64)attribute->rawvalue[2] << 16 | \
(UInt64)attribute->rawvalue[3] << 24 | \
(UInt64)attribute->rawvalue[4] << 32 | \
(UInt64)attribute->rawvalue[5] << 40

#define GetLocalizedAttributeName(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:@"SmartAttributes"]

static NSMutableDictionary *    gATASmartInterfaceWrapperCache = nil;
static NSMutableDictionary *    gATASmartAttributeOverrideCache = nil;
static NSArray *                gATASmartAttributeOverrideDatabase = nil;

@implementation HWMATASmartInterfaceWrapper

@synthesize pluginInterface = _pluginInterface;
@synthesize smartInterface = _smartInterface;
@synthesize attributes = _attributes;

+(HWMATASmartInterfaceWrapper*)wrapperWithService:(io_service_t)service productName:(NSString*)productName firmware:(NSString*)firmware bsdName:(NSString*)bsdName isRotational:(BOOL)rotational
{

    if (!gATASmartInterfaceWrapperCache) {
        gATASmartInterfaceWrapperCache = [[NSMutableDictionary alloc] init];
    }

    HWMATASmartInterfaceWrapper *wrapper = [gATASmartInterfaceWrapperCache objectForKey:bsdName];

    if (!wrapper) {
        IOCFPlugInInterface ** pluginInterface = NULL;
        IOATASMARTInterface ** smartInterface = NULL;
        SInt32 score = 0;
        int tryCount = 4;

        do {

            IOReturn result;

            if (kIOReturnSuccess == (result = IOCreatePlugInInterfaceForService(service, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score))) {

                HRESULT hresult;

                if (S_OK == (hresult = (*pluginInterface)->QueryInterface(pluginInterface, CFUUIDGetUUIDBytes(kIOATASMARTInterfaceID), (LPVOID)&smartInterface))) {

                    wrapper = [[HWMATASmartInterfaceWrapper alloc] init];

                    wrapper.pluginInterface = pluginInterface;
                    wrapper.smartInterface  = smartInterface;
                    wrapper.bsdName = bsdName;
                    wrapper.product = productName;
                    wrapper.firmware = firmware;
                    wrapper.isRotational = rotational;

                    if ([wrapper readSMARTDataAndThresholds]) {
                        [gATASmartInterfaceWrapperCache setObject:wrapper forKey:bsdName];
                        break;
                    }
                }
                else {
                    NSLog(@"pluginInterface->QueryInterface error: %d", hresult);
                }
            }
            else {
                NSLog(@"IOCreatePlugInInterfaceForService error: %@", [NSString stringFromReturn:result]);
            }

            if (smartInterface) {
                (*smartInterface)->Release(smartInterface);
            }

            if (pluginInterface) {
                IODestroyPlugInInterface(pluginInterface);
            }

            [NSThread sleepForTimeInterval:0.25];

        } while (--tryCount);

    }

    return wrapper;
}

+(HWMATASmartInterfaceWrapper*)getWrapperForBsdName:(NSString*)name
{
    if (name && gATASmartInterfaceWrapperCache) {
        return [gATASmartInterfaceWrapperCache objectForKey:name];
    }

    return nil;
}

+(void)destroyAllWrappers
{
    gATASmartInterfaceWrapperCache = nil;
}

+(NSString *)getDefaultAttributeNameByIdentifier:(NSUInteger)identifier isRotational:(BOOL)hdd
{
    BOOL ssd = !hdd;
    NSString * Unknown_HDD_Attribute = @"Unknown_HDD_Attribute";
    NSString * Unknown_SSD_Attribute = @"Unknown_SSD_Attribute";

    switch (identifier) {
        case 1:
            return @"Raw_Read_Error_Rate";
        case 2:
            return @"Throughput_Performance";
        case 3:
            return @"Spin_Up_Time";
        case 4:
            return @"Start_Stop_Count";
        case 5:
            return @"Reallocated_Sector_Ct";
        case 6:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Read_Channel_Margin";
        case 7:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Seek_Error_Rate";
        case 8:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Seek_Time_Performance";
        case 9:
            return @"Power_On_Hours";
        case 10:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Spin_Retry_Count";
        case 11:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Calibration_Retry_Count";
        case 12:
            return @"Power_Cycle_Count";
        case 13:
            return @"Read_Soft_Error_Rate";
        case 170:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Available_Reservd_Space";
        case 171:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Program_Fail_Count";
        case 173:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Wear_Leveling_Count";
        case 172:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Erase_Fail_Count";
        case 174:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Unexpect_Power_Loss_Ct";
        case 175:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Program_Fail_Count_Chip";
        case 176:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Erase_Fail_Count_Chip";
        case 177:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Wear_Leveling_Count";
        case 178:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Used_Rsvd_Blk_Cnt_Chip";
        case 179:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Used_Rsvd_Blk_Cnt_Tot";
        case 180:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Unused_Rsvd_Blk_Cnt_Tot";
        case 181:
            return @"Program_Fail_Cnt_Total";
        case 182:
            if (hdd) return Unknown_HDD_Attribute;
            return @"Erase_Fail_Count_Total";
        case 183:
            return @"Runtime_Bad_Block";
        case 184:
            return @"End-to-End_Error";
        case 187:
            return @"Reported_Uncorrect";
        case 188:
            return @"Command_Timeout";
        case 189:
            if (ssd) return @"Factory_Bad_Block_Ct";
            return @"High_Fly_Writes";
        case 190:
            // Western Digital uses this for temperature.
            // It's identical to Attribute 194 except that it
            // has a failure threshold set to correspond to the
            // max allowed operating temperature of the drive, which
            // is typically 55C.  So if this attribute has failed
            // in the past, it indicates that the drive temp exceeded
            // 55C sometime in the past.
            return @"Airflow_Temperature_Cel";
        case 191:
            if (ssd) return Unknown_SSD_Attribute;
            return @"G-Sense_Error_Rate";
        case 192:
            return @"Power-Off_Retract_Count";
        case 193:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Load_Cycle_Count";
        case 194:
            return @"Temperature_Celsius";
        case 195:
            // Fujitsu: "ECC_On_The_Fly_Count";
            return @"Hardware_ECC_Recovered";
        case 196:
            return @"Reallocated_Event_Count";
        case 197:
            return @"Current_Pending_Sector";
        case 198:
            return @"Offline_Uncorrectable";
        case 199:
            return @"UDMA_CRC_Error_Count";
        case 200:
            if (ssd) return Unknown_SSD_Attribute;
            // Western Digital
            return @"Multi_Zone_Error_Rate";
        case 201:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Soft_Read_Error_Rate";
        case 202:
            if (ssd) return @"Perc_Rated_Life_Used";
            // Fujitsu: "TA_Increase_Count"
            return @"Data_Address_Mark_Errs";
        case 203:
            // Fujitsu
            return @"Run_Out_Cancel";
            // Maxtor: ECC Errors
        case 204:
            // Fujitsu: "Shock_Count_Write_Opern"
            return @"Soft_ECC_Correction";
        case 205:
            // Fujitsu: "Shock_Rate_Write_Opern"
            return @"Thermal_Asperity_Rate";
        case 206:
            // Fujitsu
            if (ssd) return Unknown_SSD_Attribute;
            return @"Flying_Height";
        case 207:
            // Maxtor
            if (ssd) return Unknown_SSD_Attribute;
            return @"Spin_High_Current";
        case 208:
            // Maxtor
            if (ssd) return Unknown_SSD_Attribute;
            return @"Spin_Buzz";
        case 209:
            // Maxtor
            if (ssd) return Unknown_SSD_Attribute;
            return @"Offline_Seek_Performnce";
        case 220:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Disk_Shift";
        case 221:
            if (ssd) return Unknown_SSD_Attribute;
            return @"G-Sense_Error_Rate";
        case 222:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Loaded_Hours";
        case 223:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Load_Retry_Count";
        case 224:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Load_Friction";
        case 225:
            if (ssd) return @"Host_Writes_32MiB";
            return @"Load_Cycle_Count";
        case 226:
            if (ssd) return @"Workld_Media_Wear_Indic";
            return @"Load-in_Time";
        case 227:
            if (ssd) return @"Workld_Host_Reads_Perc";
            return @"Torq-amp_Count";
        case 228:
            return @"Power-off_Retract_Count";
        case 230:
            // seen in IBM DTPA-353750
            if (ssd) return @"Life_Curve_Status";
            return @"Head_Amplitude";
        case 231:
            if (ssd) return @"SSD_Life_Left";
            return @"Temperature_Celsius";
        case 232:
            // seen in Intel X25-E SSD
            return @"Available_Reservd_Space";
        case 233:
            // seen in Intel X25-E SSD
            if (hdd) return Unknown_HDD_Attribute;
            return @"Media_Wearout_Indicator";
        case 240:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Head_Flying_Hours";
        case 241:
            return @"Total_LBAs_Written";
        case 242:
            return @"Total_LBAs_Read";
        case 250:
            return @"Read_Error_Retry_Rate";
        case 254:
            if (ssd) return Unknown_SSD_Attribute;
            return @"Free_Fall_Sensor";
        default:
            return @"Unknown_Attribute";
    }
}

+(NSString*)getDefaultRawFormatForIdentifier:(NSUInteger)identifier
{
    switch (identifier) {
        case 3:   // Spin-up time
            return @"raw16(avg16)";

        case 5:   // Reallocated sector count
        case 196: // Reallocated event count
            return @"raw16(raw16)";

        case 9:  // Power on hours
            return @"raw24(raw8)";

        case 190: // Temperature
        case 194:
            return @"tempminmax";
            
        default:
            return @"raw48";
    }
}

+(unsigned long long)getRawValueForAttribute:(ATASMARTAttribute*)attribute format:(NSString*)format
{
    NSArray *options = [format componentsSeparatedByString:@":"];

    NSString *type = options.count ? options[0] : format;
    NSString *byteorder = options.count > 1 ? options[1] : nil;

    if (!byteorder) {

        if ([type isEqualToString:@"raw64"] || [type isEqualToString:@"hex64"]) {
            byteorder = @"543210wv";
        }
        else if ([type isEqualToString:@"raw56"] || [type isEqualToString:@"hex56"] ||
                 [type isEqualToString:@"raw24/raw32"] || [type isEqualToString:@"msec24hour32"]) {
            byteorder = @"r543210";
        }
        else {
            byteorder = @"543210";
        }
    }

    // Build 64-bit value from selected bytes
    unsigned long long rawvalue = 0;

    for (int i = 0; i < byteorder.length; i++) {

        unsigned char b;

        switch ([byteorder characterAtIndex:i]) {
            case '0': b = attribute->rawvalue[0];  break;
            case '1': b = attribute->rawvalue[1];  break;
            case '2': b = attribute->rawvalue[2];  break;
            case '3': b = attribute->rawvalue[3];  break;
            case '4': b = attribute->rawvalue[4];  break;
            case '5': b = attribute->rawvalue[5];  break;
            case 'r': b = attribute->reserv;  break;
            case 'v': b = attribute->current; break;
            case 'w': b = attribute->worst;   break;
            default : b = 0;            break;
        }
        rawvalue <<= 8; rawvalue |= b;
    }
    
    return rawvalue;
}

+(NSString*)getFormattedRawValueForAttribute:(ATASMARTAttribute*)attribute format:(NSString*)format
{
    // Get 48 bit or 64 bit raw value
    unsigned long long rawvalue = [HWMATASmartInterfaceWrapper getRawValueForAttribute:attribute format:format];

    // Split into bytes and words
    unsigned char raw[6];
    raw[0] = (unsigned char) rawvalue;
    raw[1] = (unsigned char)(rawvalue >>  8);
    raw[2] = (unsigned char)(rawvalue >> 16);
    raw[3] = (unsigned char)(rawvalue >> 24);
    raw[4] = (unsigned char)(rawvalue >> 32);
    raw[5] = (unsigned char)(rawvalue >> 40);
    unsigned word[3];
    word[0] = raw[0] | (raw[1] << 8);
    word[1] = raw[2] | (raw[3] << 8);
    word[2] = raw[4] | (raw[5] << 8);


    if ([format isEqualToString:@"raw8"]) {
        return [NSString stringWithFormat:@"%d %d %d %d %d %d", raw[5], raw[4], raw[3], raw[2], raw[1], raw[0]];
    }
    else if ([format isEqualToString:@"raw16"]) {
        return [NSString stringWithFormat:@"%u %u %u", word[2], word[1], word[0]];
    }
    else if ([format isEqualToString:@"raw48"] ||
             [format isEqualToString:@"raw48"] || [format isEqualToString:@"raw64"]) {
        return [NSString stringWithFormat:@"%llu", rawvalue];
    }
    else if ([format isEqualToString:@"hex48"]) {
        return [NSString stringWithFormat:@"0x%012llx", rawvalue];
    }
    else if ([format isEqualToString:@"hex56"]) {
        return [NSString stringWithFormat:@"0x%014llx", rawvalue];
    }
    else if ([format isEqualToString:@"hex64"]) {
        return [NSString stringWithFormat:@"0x%016llx", rawvalue];
    }
    else if ([format isEqualToString:@"raw16(raw16)"]) {
        NSString *s = [NSString stringWithFormat:@"%u", word[0]];
        return word[1] || word[2] ? [s stringByAppendingString:[NSString stringWithFormat:@" (%u %u)", word[1], word[2]]] : s;
    }
    else if ([format isEqualToString:@"raw16(avg16)"]) {
        NSString *s = [NSString stringWithFormat:@"%u", word[0]];
        return word[1] ? [s stringByAppendingString:[NSString stringWithFormat:GetLocalizedAttributeName(@" (Average %u)"), word[1]]] : s;
    }
    else if ([format isEqualToString:@"raw24(raw8)"]) {
        NSString *s = [NSString stringWithFormat:@"%u", (unsigned)(rawvalue & 0x00ffffffULL)];
        return raw[3] || raw[4] || raw[5] ? [s stringByAppendingString:[NSString stringWithFormat:@" (%d %d %d)", raw[5], raw[4], raw[3]]] : s;
    }
    else if ([format isEqualToString:@"raw24/raw24"]) {
        return [NSString stringWithFormat:@"%u/%u", (unsigned)(rawvalue >> 24), (unsigned)(rawvalue & 0x00ffffffULL)];
    }
    else if ([format isEqualToString:@"raw24/raw32"]) {
        return [NSString stringWithFormat:@"%u/%u", (unsigned)(rawvalue >> 32), (unsigned)(rawvalue & 0xffffffffULL)];
    }
    else if ([format isEqualToString:@"min2hour"]) {
        // minutes
        unsigned long long temp = word[0]+(word[1]<<16);
        unsigned long long tmp1 = temp/60;
        unsigned long long tmp2 = temp%60;
        NSString *s = [NSString stringWithFormat:@"%lluh+%02llum", tmp1, tmp2];
        return word[2] ? [s stringByAppendingString:[NSString stringWithFormat:GetLocalizedAttributeName(@" (%u)"), word[2]]] : s;
    }
    else if ([format isEqualToString:@"sec2hour"]) {
        // seconds
        int64_t hours = rawvalue/3600;
        int64_t minutes = (rawvalue-3600*hours)/60;
        int64_t seconds = rawvalue%60;
        return [NSString stringWithFormat:@"%lluh+%02llum+%0llxus", hours, minutes, seconds];
    }
    else if ([format isEqualToString:@"halfmin2hour"]) {
        // 30-second counter
        int64_t hours = rawvalue/120;
        int64_t minutes = (rawvalue-120*hours)/2;
        return [NSString stringWithFormat:@"%lluh+%02llum", hours, minutes];
    }
    else if ([format isEqualToString:@"msec24hour32"]) {
        // hours + milliseconds
        unsigned hours = (unsigned)(rawvalue & 0xffffffffULL);
        unsigned milliseconds = (unsigned)(rawvalue >> 32);
        unsigned seconds = milliseconds / 1000;
        return [NSString stringWithFormat:@"%uh+%02um+%02u.%03us",
                hours, seconds / 60, seconds % 60, milliseconds % 1000];
    }
    else if ([format isEqualToString:@"tempminmax"]) {
        // Search for possible min/max values
        // 00 HH 00 LL 00 TT (Hitachi/IBM)
        // 00 00 HH LL 00 TT (Maxtor, Samsung)
        // 00 00 00 HH LL TT (WDC)
        unsigned char lo = 0, hi = 0;
        int cnt = 0;
        for (int i = 1; i < 6; i++) {
            if (raw[i])
                switch (cnt++) {
                    case 0:
                        lo = raw[i];
                        break;
                    case 1:
                        if (raw[i] < lo) {
                            hi = lo; lo = raw[i];
                        }
                        else
                            hi = raw[i];
                        break;
                }
        }

        unsigned char t = raw[0];
        if (cnt == 0)
            return [NSString stringWithFormat:@"%d", t];
        else if (cnt == 2 && 0 < lo && lo <= t && t <= hi && hi < 128)
            return [NSString stringWithFormat:@"%d (Min/Max %d/%d)", t, lo, hi];
        else
            return [NSString stringWithFormat:@"%d (%d %d %d %d %d)", t, raw[5], raw[4], raw[3], raw[2], raw[1]];
    }
    else if ([format isEqualToString:@"temp10x"]) {
        return [NSString stringWithFormat:@"%d.%d", word[0]/10, word[0]%10];
    }

    return @"?"; // Should not happen
}

+(NSDictionary*)getAttributeOverrideForProduct:(NSString*)product firmware:(NSString*)firmware
{
    if (!product)
        return nil;

    if (!gATASmartAttributeOverrideDatabase) {
        if (!(gATASmartAttributeOverrideDatabase = [NSArray arrayWithContentsOfURL:[[NSBundle mainBundle] URLForResource:@"SmartOverrides" withExtension:@"plist"]])) {
            gATASmartAttributeOverrideDatabase = [NSArray array]; // Empty dictionary
        }
    }

    if (!gATASmartAttributeOverrideCache) {
        gATASmartAttributeOverrideCache = [[NSMutableDictionary alloc] init];
    }

    NSString *identifier = [NSString stringWithFormat:@"%@%@", product, firmware];

    NSDictionary *overrides = gATASmartAttributeOverrideCache[identifier];

    if (!overrides) {

        for (NSDictionary *group in gATASmartAttributeOverrideDatabase) {

            NSArray *productMatch = group[@"NameMatch"];

            for (NSString *productPattern in productMatch) {

                NSRegularExpression *productExpression = [NSRegularExpression regularExpressionWithPattern:productPattern options:NSRegularExpressionDotMatchesLineSeparators error:nil];

                if ([productExpression numberOfMatchesInString:product options:NSMatchingReportCompletion range:NSMakeRange(0, product.length)]) {

                    BOOL firmwareSupported = YES;

                    NSArray *firmwareMatch = group[@"FirmwareMatch"];

                    if (firmware && firmwareMatch) {

                        BOOL supported = NO;

                        for (NSString *firmwarePattern in firmwareMatch) {

                            NSRegularExpression *firmwareExpression = [NSRegularExpression regularExpressionWithPattern:firmwarePattern options:NSRegularExpressionDotMatchesLineSeparators error:nil];

                            if ([firmwareExpression numberOfMatchesInString:firmware options:NSMatchingReportCompletion range:NSMakeRange(0, firmware.length)]) {
                                supported = YES;
                                break;
                            }
                        }

                        firmwareSupported = supported;
                    }

                    if (firmwareSupported) {
                        __block NSMutableDictionary *arranged = [NSMutableDictionary dictionary];

                        [group[@"Attributes"] enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
                            NSArray *idAndFormat = [key componentsSeparatedByString:@","];

                            [arranged setObject:@{@"name": obj, @"format": idAndFormat[1]} forKey:idAndFormat[0]];
                        }];

                        overrides = [arranged copy];

                        [gATASmartAttributeOverrideCache setObject:overrides forKey:identifier];
                        
                        break;
                    }
                }
            }

        }
    }

    return overrides ? overrides : @{};
}

-(BOOL)readSMARTDataAndThresholds
{
    NSLog(@"reading SMART data for %@", _product);

    IOReturn result = kIOReturnError;

    ATASMARTData smartData;
    ATASMARTDataThresholds smartDataThresholds;

    bzero(&smartData, sizeof(smartData));
    bzero(&smartDataThresholds, sizeof(smartDataThresholds));

    Boolean exceeded = false;

    if (kIOReturnSuccess != (*_smartInterface)->SMARTReturnStatus(_smartInterface, &exceeded)) {
        if (kIOReturnSuccess != (*_smartInterface)->SMARTEnableDisableOperations(_smartInterface, true)) {
            if (kIOReturnSuccess != (*_smartInterface)->SMARTEnableDisableAutosave(_smartInterface, true)) {
                NSLog(@"SMARTEnableDisableAutosave returned error for: %@ code: %@", _product, [NSString stringFromReturn:result]);
            }
        }
    }

    if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTReturnStatus(_smartInterface, &exceeded))) {
        if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTReadData(_smartInterface, &smartData))) {
            if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTValidateReadData(_smartInterface, &smartData))) {

                bcopy(&smartData.vendorSpecific1, &_vendorSpecificData, sizeof(_vendorSpecificData));

                if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTReadDataThresholds(_smartInterface, &smartDataThresholds))) {
                    if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTValidateReadData(_smartInterface, (ATASMARTData*)&smartDataThresholds))) {
                        bcopy(&smartDataThresholds.vendorSpecific1, &_vendorSpecificThresholds, sizeof(_vendorSpecificThresholds));
                    }
                    else NSLog(@"SMARTValidateReadData after SMARTReadDataThresholds returned error for: %@ code: %@", _product, [NSString stringFromReturn:result]);
                }
                else NSLog(@"SMARTReadDataThresholds returned error for: %@ code: %@", _product, [NSString stringFromReturn:result]);

                // Prepare SMART attributes list
                if (!gATASmartAttributeOverrideCache) {
                    gATASmartAttributeOverrideCache = [[NSMutableDictionary alloc] init];
                }

                _overrides = [HWMATASmartInterfaceWrapper getAttributeOverrideForProduct:_product firmware:_firmware];

                NSMutableArray * attributes = [[NSMutableArray alloc] init];

                NSUInteger count = 0;

                for (int index = 0; index < kATASMARTAttributesCount; index++) {

                    ATASMARTAttribute *attribute = &_vendorSpecificData.vendorAttributes[index];

                    if (attribute->attributeId) {

                        ATASmartThresholdAttribute *threshold = &_vendorSpecificThresholds.ThresholdEntries[index];

                        NSString *overriddenName = _overrides ? [_overrides objectForKey:[NSString stringWithFormat:@"%d",attribute->attributeId]][@"name"] : nil;
                        NSString *overriddenFormat = _overrides ? [_overrides objectForKey:[NSString stringWithFormat:@"%d",attribute->attributeId]][@"format"] : nil;

                        NSString *name = overriddenName ? overriddenName : [HWMATASmartInterfaceWrapper getDefaultAttributeNameByIdentifier:attribute->attributeId isRotational:_rotational];
                        NSString *format = overriddenFormat ? overriddenFormat : [HWMATASmartInterfaceWrapper getDefaultRawFormatForIdentifier:attribute->attributeId];

                        NSString *title = GetLocalizedAttributeName(name);

                        NSInteger level = kHWMSensorLevelDisabled;

                        if (threshold && attribute->current && threshold->ThresholdValue)
                        {
                            if (attribute->current < threshold->ThresholdValue) {
                                level = kHWMSensorLevelExceeded;
                            }
                            else if ((float)threshold->ThresholdValue / (float)attribute->current > 0.99) {
                                level = kHWMSensorLevelHigh;
                            }
                            else {
                                level = kHWMSensorLevelNormal;
                            }
                        }

                        if (ATTRIBUTE_FLAGS_PREFAILURE(attribute->flag) && [HWMEngine sharedEngine] && [HWMEngine sharedEngine].configuration.notifyAlarmLevelChanges) {
                            switch (level) {
                                case kHWMSensorLevelExceeded:
                                    [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                                                description:[NSString stringWithFormat:GetLocalizedString(@"'%@' S.M.A.R.T. attribute is critical for %@. Drive failure predicted!"), title, _product]
                                                           notificationName:NotifierSensorLevelExceededNotification
                                                                   iconData:nil
                                                                   priority:0
                                                                   isSticky:YES
                                                               clickContext:nil];
                                    break;

                                default:
                                    // none
                                    break;
                            }
                        }

                        [attributes addObject:@{@"level": [NSNumber numberWithInteger:level],
                                                @"index" : [NSNumber numberWithUnsignedInteger:count++],
                                                @"id": [NSNumber numberWithUnsignedChar:attribute->attributeId],
                                                @"name": name,
                                                @"title": title,
                                                @"critical": GetLocalizedString(ATTRIBUTE_FLAGS_PREFAILURE(attribute->flag) ? @"Pre-Failure" : @"Life-Span"),
                                                @"value": [NSNumber numberWithUnsignedChar:attribute->current],
                                                @"worst": [NSNumber numberWithUnsignedChar:attribute->worst],
                                                @"threshold": (threshold ? [NSNumber numberWithUnsignedChar:threshold->ThresholdValue] : @0),
                                                @"raw": [NSNumber numberWithUnsignedLongLong:RAW_TO_LONG(attribute)],
                                                @"rawFormatted": [HWMATASmartInterfaceWrapper getFormattedRawValueForAttribute:attribute format:format],
                                               }];
                    }
                }

                _attributes = [attributes copy];
            }
            else NSLog(@"SMARTValidateReadData after SMARTReadData returned error for: %@ code: %@", _product, [NSString stringFromReturn:result]);
        }
        else NSLog(@"SMARTReadData returned error for: %@ code: %@", _product, [NSString stringFromReturn:result]);
    }
    else NSLog(@"SMARTReturnStatus returned error for: %@ code: %@", _product, [NSString stringFromReturn:result]);

    return result == kIOReturnSuccess;
}

-(void)releaseInterface
{
    if (self.smartInterface) {
        (*self.smartInterface)->Release(self.smartInterface);
    }

    if (self.pluginInterface) {
        IODestroyPlugInInterface(self.pluginInterface);
    }
}

-(void)dealloc
{
    [self releaseInterface];
}

@end

#pragma mark
#pragma mark HWMAtaSmartSensor

static void block_device_appeared(void *engine, io_iterator_t iterator);
static void block_device_disappeared(void *engine, io_iterator_t iterator);

@implementation HWMAtaSmartSensor

@dynamic bsdName;
@dynamic productName;
@dynamic volumeNames;
@dynamic revision;
@dynamic serialNumber;
@dynamic rotational;

@synthesize attributes = _attributes;

static NSDictionary * gAvailableMountedPartitions = nil;

static IONotificationPortRef gHWMAtaSmartSensorNotificationPort = MACH_PORT_NULL;
static io_iterator_t gHWMAtaSmartDeviceIterator = 0;

+(void)stopWatchingForBlockStorageDevices
{
    if (gHWMAtaSmartSensorNotificationPort != MACH_PORT_NULL) {
        CFRunLoopRemoveSource(CFRunLoopGetMain(), IONotificationPortGetRunLoopSource(gHWMAtaSmartSensorNotificationPort), kCFRunLoopDefaultMode);

        IONotificationPortDestroy(gHWMAtaSmartSensorNotificationPort);

        gHWMAtaSmartSensorNotificationPort = MACH_PORT_NULL;
    }

    IOObjectRelease(gHWMAtaSmartDeviceIterator);
}

+ (void)startWatchingForBlockStorageDevicesWithEngine:(HWMEngine *)engine
{
    [HWMAtaSmartSensor stopWatchingForBlockStorageDevices];

    // Add notification ports to runloop
    gHWMAtaSmartSensorNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);

    CFRunLoopAddSource([[NSRunLoop currentRunLoop] getCFRunLoop], IONotificationPortGetRunLoopSource(gHWMAtaSmartSensorNotificationPort), kCFRunLoopDefaultMode);

    [HWMAtaSmartSensor discoverDevicesWithEngine:engine matching:IOServiceMatching("IOBlockStorageDevice")];
}

+ (void)discoverDevicesWithEngine:(HWMEngine *)engine matching:(CFDictionaryRef)matching
{
    // Retain matching dictionary so it will stay also while registering kIOTerminatedNotification
    CFRetain(matching);

    // Discover devices and add notification callbacks
    if (!IOServiceAddMatchingNotification(gHWMAtaSmartSensorNotificationPort,
                                          kIOMatchedNotification,
                                          matching,
                                          block_device_appeared,
                                          (__bridge void *)engine,
                                          &gHWMAtaSmartDeviceIterator))
    {
        // Add matched devices
        block_device_appeared((__bridge void*)engine, gHWMAtaSmartDeviceIterator);

        if (!IOServiceAddMatchingNotification(gHWMAtaSmartSensorNotificationPort,
                                              kIOTerminatedNotification,
                                              matching,
                                              block_device_disappeared,
                                              (__bridge void *)engine,
                                              &gHWMAtaSmartDeviceIterator)) {

            while (IOIteratorNext(gHWMAtaSmartDeviceIterator)) {};
        }
    }
}

+(void)updatePartitionsList
{
    NSMutableDictionary *partitions = [[NSMutableDictionary alloc] init];

    BOOL first = YES;

    NSArray *mountedVolumeURLs = [[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:@[] options:0];

    for (NSURL *url in mountedVolumeURLs) {
        NSString *path = url.path;
        struct statfs buffer;

        if (statfs([path fileSystemRepresentation], &buffer) == 0)
        {
            NSRange start = [path rangeOfString:@"/Volumes/"];

            if (first == NO && start.length == 0)
            {
                continue;
            }

            if (first)
                first = NO;

            NSString *name = [[NSString stringWithFormat:@"%s",buffer.f_mntfromname] lastPathComponent];

            if ([name hasPrefix:@"disk"] && [name length] > 4)
            {
                NSString *newName = [name substringFromIndex:4];
                NSRange paritionLocation = [newName rangeOfString:@"s"];

                if(paritionLocation.length != 0)
                    name = [NSString stringWithFormat:@"disk%@",[newName substringToIndex: paritionLocation.location]];
            }

            if( [partitions objectForKey:name] )
                [[partitions objectForKey:name] addObject:[[NSFileManager defaultManager] displayNameAtPath:path]];
            else
                [partitions setObject:[[NSMutableArray alloc] initWithObjects:[[NSFileManager defaultManager] displayNameAtPath:path], nil] forKey:name];
        }
    }

    gAvailableMountedPartitions = [partitions copy];
}

+(NSDictionary*)partitions
{
    if (!gAvailableMountedPartitions) {
        [HWMAtaSmartSensor updatePartitionsList];
    }

    return gAvailableMountedPartitions;
}

-(NSString *)title
{
    switch (self.engine.configuration.driveNameSelector.unsignedIntegerValue) {
        case kHWMDriveNameVolumes:
            return self.volumeNames;

        case kHWMDriveNameBSD:
            return self.bsdName;

        case kHWMDriveNameProduct:
        default:
            break;
    }

    return self.productName;
}

-(NSString *)legend
{
    switch (self.engine.configuration.driveLegendSelector.unsignedIntegerValue) {
        case kHWMDriveNameProduct:
            return self.productName;

        case kHWMDriveNameBSD:
            return self.bsdName;

        case kHWMDriveNameVolumes:
        default:
            break;
    }

    return self.volumeNames;
}

-(void)initialize
{
    _temperatureAttributeIndex = -1;
    _remainingLifeAttributeIndex = -1;

    [self addObserver:self forKeyPath:@keypath(self, engine.configuration.driveNameSelector) options:0 context:nil];
    [self addObserver:self forKeyPath:@keypath(self, engine.configuration.driveLegendSelector) options:0 context:nil];
}

-(void)awakeFromFetch
{
    [super awakeFromFetch];
    [self initialize];
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];
    [self initialize];
}

-(void)prepareForDeletion
{
    [super prepareForDeletion];

    [self removeObserver:self forKeyPath:@keypath(self, engine.configuration.driveNameSelector)];
    [self removeObserver:self forKeyPath:@keypath(self, engine.configuration.driveLegendSelector)];

    IOObjectRelease((io_service_t)self.service.unsignedLongLongValue);
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@keypath(self, engine.configuration.driveNameSelector)]) {
        [self willChangeValueForKey:@keypath(self, title)];
        [self didChangeValueForKey:@keypath(self, title)];
    }
    else if ([keyPath isEqualToString:@keypath(self, engine.configuration.driveLegendSelector)]) {
        [self willChangeValueForKey:@keypath(self, legend)];
        [self didChangeValueForKey:@keypath(self, legend)];
    }

    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}


-(BOOL)findIndexOfAttributeByName:(NSString*)name outIndex:(NSInteger*)index
{
    NSArray *results = [_attributes filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name = %@", name]];

    *index = results && results.count ? [[results objectAtIndex:0][@"index"] unsignedIntegerValue] : -1;

    return *index > -1;
}

-(void)updateVolumeNames
{
    NSString *volumes = [[[HWMAtaSmartSensor partitions] objectForKey:self.bsdName] componentsJoinedByString:@", "];

    if (!volumes) {
        NSString *bsdUnit = bsdname_from_service((io_registry_entry_t)self.service.unsignedLongLongValue);
        volumes = [[[HWMAtaSmartSensor partitions] objectForKey:bsdUnit] componentsJoinedByString:@", "];
    }

    [self setPrimitiveValue:volumes ? volumes : self.bsdName forKey:@"volumeNames"];
}

-(NSNumber*)getTemperature
{
    if (_temperatureAttributeIndex < 0) {

        if (![self findIndexOfAttributeByName:@"Temperature_Celsius" outIndex:&_temperatureAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Airflow_Temperature_Cel" outIndex:&_temperatureAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Temperature_Case" outIndex:&_temperatureAttributeIndex])
        {
            return nil;
        }
    }

    if (!_attributes || _temperatureAttributeIndex >= _attributes.count)
        return nil;

    NSNumber *raw = [_attributes objectAtIndex:_temperatureAttributeIndex][@"raw"];

    return [NSNumber numberWithUnsignedShort:raw.unsignedShortValue];
}

-(NSNumber*)getRemainingLife
{
    if (_remainingLifeAttributeIndex < 0) {

        if (![self findIndexOfAttributeByName:@"SSD_Life_Left" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Remaining_Lifetime_Perc" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Perc_Rated_Life_Used" outIndex:&_remainingLifeAttributeIndex] &&
            /*![self findIndexOfAttributeByName:@"Wear_Leveling_Count" outIndex:&_remainingLifeAttributeIndex] &&*/
            ![self findIndexOfAttributeByName:@"Media_Wearout_Indicator" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Available_Reservd_Space" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Reserve_Block_Count" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Bad_Block_Count" outIndex:&_remainingLifeAttributeIndex])
        {
            return nil;
        }
    }

    if (!self.attributes || _remainingLifeAttributeIndex >= self.attributes.count)
        return nil;

    return [self.attributes objectAtIndex:_remainingLifeAttributeIndex][@"value"];
}

-(NSUInteger)internalUpdateAlarmLevel
{
    float floatValue = self.value.floatValue;

    switch (self.selector.unsignedIntegerValue) {

        case kHWMGroupSmartTemperature:
            if (self.rotational.boolValue) {
                return  floatValue >= 55 ? kHWMSensorLevelExceeded :
                floatValue >= 50 ? kHWMSensorLevelHigh :
                floatValue >= 40 ? kHWMSensorLevelModerate :
                kHWMSensorLevelNormal;
            }
            else {
                return  floatValue >= 100 ? kHWMSensorLevelExceeded :
                floatValue >= 85 ? kHWMSensorLevelHigh :
                floatValue >= 70 ? kHWMSensorLevelModerate :
                kHWMSensorLevelNormal;
            }

        case kHWMGroupSmartRemainingLife:
            return  floatValue < 10 ? kHWMSensorLevelExceeded :
            floatValue < 50 ? kHWMSensorLevelHigh :
            floatValue < 70 ? kHWMSensorLevelModerate :
            kHWMSensorLevelNormal;

        default:
            break;
    }

    return _alarmLevel;
}

-(NSNumber *)internalUpdateValue
{
    if (self.hidden.boolValue)
        return nil;

    HWMATASmartInterfaceWrapper *wrapper = [HWMATASmartInterfaceWrapper getWrapperForBsdName:self.bsdName];

    if (!wrapper) {

        wrapper = [HWMATASmartInterfaceWrapper wrapperWithService:(io_service_t)self.service.unsignedLongLongValue
                                                         productName:self.productName
                                                            firmware:self.revision
                                                             bsdName:self.bsdName
                                                        isRotational:self.rotational.boolValue];
    }

    _attributes = wrapper ? [wrapper.attributes copy] : nil;

    switch (self.selector.unsignedIntegerValue) {
        case kHWMGroupTemperature:
        case kHWMGroupSmartTemperature:
            return [self getTemperature];

        case kHWMGroupSmartRemainingLife:
            return [self getRemainingLife];
            
    }

    return @0;
}

-(void)internalSendAlarmNotification
{
    if (self.selector.unsignedIntegerValue == kHWMGroupSmartRemainingLife) {
        switch (_alarmLevel) {
            case kHWMSensorLevelExceeded:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ is exceeded limit"), self.title]
                                       notificationName:NotifierSensorLevelExceededNotification
                                               iconData:nil
                                               priority:1000
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelHigh:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ is critical"), self.title]
                                       notificationName:NotifierSensorLevelHighNotification
                                               iconData:nil
                                               priority:500
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelModerate:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ lifespan is low"), self.title]
                                       notificationName:NotifierSensorLevelModerateNotification
                                               iconData:nil
                                               priority:0
                                               isSticky:YES
                                           clickContext:nil];
                break;
                
            default:
                break;
        }
    }
    else {
        [super internalSendAlarmNotification];
    }
}

@end

NSString* bsdname_from_service(io_registry_entry_t object)
{
    CFMutableDictionaryRef propertiesRef;

    if (KERN_SUCCESS == IORegistryEntryCreateCFProperties(object, &propertiesRef, kCFAllocatorDefault, 0)) {

        NSDictionary *properties = CFBridgingRelease(propertiesRef);

        NSString *bsdName = [properties[@"BSD Name"] copy];
        NSNumber *open = properties[@"Open"];
        NSNumber *leaf = properties[@"Leaf"];
        NSNumber *whole = properties[@"Whole"];

        if (bsdName &&
            open && open.boolValue &&
            leaf && leaf.boolValue &&
            whole && whole.boolValue) {
            return bsdName;
        }
    }

    io_iterator_t childIterator;

    if (KERN_SUCCESS == IORegistryEntryGetChildIterator(object, kIOServicePlane, &childIterator)) {

        io_registry_entry_t child;
        NSString *bsdName = nil;

        while ((child = IOIteratorNext(childIterator))) {

            bsdName = bsdname_from_service(child);

            IOObjectRelease(child);

            if (bsdName) {
                break;
            }
        }

        IOObjectRelease(childIterator);

        return bsdName;
    }

    return nil;
}

static void block_device_appeared(void *engine, io_iterator_t iterator)
{
    dispatch_async(dispatch_get_main_queue(), ^{

        io_object_t object;

        __block NSMutableArray *devices = [NSMutableArray array];

        while ((object = IOIteratorNext(iterator))) {

            NSLog(@"ATA block storage device appeared %u", object);

            CFMutableDictionaryRef propertiesRef;

            if (KERN_SUCCESS == IORegistryEntryCreateCFProperties(object, &propertiesRef, kCFAllocatorDefault, 0)) {

                NSDictionary *properties = CFBridgingRelease(propertiesRef);

                NSNumber *capable = properties[@kIOPropertySMARTCapableKey];

                if (capable && capable.boolValue) {

                    NSDictionary * characteristics = properties[@"Device Characteristics"];

                    if (characteristics) {
                        NSString *name = [(NSString*)characteristics[@"Product Name"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
                        NSString *medium = characteristics[@"Medium Type"];
                        NSString *revision = [(NSString*)characteristics[@"Product Revision Level"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

                        NSString *serial = [(NSString*)characteristics[@"Serial Number"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

                        // Try to obtain serial number from device nub
                        if (!serial) {
                            CFStringRef serialRef = IORegistryEntrySearchCFProperty(object, kIOServicePlane, CFSTR("device serial"), kCFAllocatorDefault, kIORegistryIterateParents | kIORegistryIterateRecursively);

                            if (MACH_PORT_NULL != serialRef) {
                                serial = CFBridgingRelease(serialRef);
                            }
                        }

                        if (name && revision) {
                            NSString *bsdName;
                            NSString *volumes;

                            CFStringRef bsdNameRef = IORegistryEntrySearchCFProperty(object, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);

                            if (MACH_PORT_NULL != bsdNameRef) {
                                bsdName = CFBridgingRelease(bsdNameRef);
                                volumes = [[[HWMAtaSmartSensor partitions] objectForKey:bsdName] componentsJoinedByString:@", "];

                                if (!volumes) {
                                    NSString *bsdUnit = bsdname_from_service(object);
                                    volumes = [[[HWMAtaSmartSensor partitions] objectForKey:bsdUnit] componentsJoinedByString:@", "];
                                }

                                if (bsdName) {
                                    [devices addObject:@{@"service" : [NSNumber numberWithUnsignedLongLong:object],
                                                         @"productName": name,
                                                         @"bsdName" :bsdName,
                                                         @"volumesNames" : (volumes ? volumes : bsdName) ,
                                                         @"serialNumber" : serial ? serial : revision,
                                                         @"revision" : revision,
                                                         @"rotational" : [NSNumber numberWithBool:medium ? ![medium isEqualToString:@"Solid State"] : TRUE]}
                                     ];
                                }
                            }
                        }
                    }
                }
            }
        }

        [(__bridge HWMEngine*)engine systemDidAddBlockStorageDevices:devices];
    });
}

static void block_device_disappeared(void *engine, io_iterator_t iterator)
{
    io_object_t object;
    __block NSMutableArray *devices = [NSMutableArray array];

    while ((object = IOIteratorNext(iterator))) {
        NSLog(@"ATA block storage device disappeared %u", object);
        
        [devices addObject:[NSNumber numberWithUnsignedLongLong:object]];
        
        IOObjectRelease(object);
    }
    
    [(__bridge HWMEngine*)engine systemDidRemoveBlockStorageDevices:devices];
}
