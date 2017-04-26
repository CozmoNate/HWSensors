//
//  HWMATASmartInterfaceWrapper.m
//  HWMonitor
//
//  Created by Natan Zalkin on 16/04/2017.
//  Copyright © 2017 kozlek. All rights reserved.
//

#import "HWMATASmartInterfaceWrapper.h"
#import "NSString+returnCodeDescription.h"

#import "HWMSensor.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"

#import "Localizer.h"

#import <Growl/Growl.h>

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
@synthesize overrides = _overrides;

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
                    
                    (*smartInterface)->AddRef(smartInterface);
                    (*pluginInterface)->AddRef(pluginInterface);
                    
                    wrapper.pluginInterface = pluginInterface;
                    wrapper.smartInterface  = smartInterface;
                    wrapper.bsdName = bsdName;
                    wrapper.product = productName;
                    wrapper.firmware = firmware;
                    wrapper.isRotational = rotational;
                    
                    if ([wrapper readSMARTDataAndThresholds]) {
                        [gATASmartInterfaceWrapperCache setObject:wrapper forKey:bsdName];
                        return wrapper;
                    }
                }
                else {
                    NSLog(@"pluginInterface->QueryInterface error: %d for %@", hresult, productName);
                }
            }
            else {
                NSLog(@"IOCreatePlugInInterfaceForService error: %@ for %@", [NSString stringFromReturn:result], productName);
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
    [gATASmartInterfaceWrapperCache removeAllObjects];
    
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
            //        case 1:   // Raw Read Error Rate
            //        case 7:   // Seek Error Rate
            //            return @"raw24/raw32";
            
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
    else if ([format isEqualToString:@"raw48"] || [format isEqualToString:@"raw64"]) {
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
    else if ([format isEqualToString:@"raw32(err16)"]) {
        return [NSString stringWithFormat:@"%u (Error %u)", (unsigned)(rawvalue & 0xffffffffULL), (unsigned)(rawvalue >> 32)];
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
                        
                        __block NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
                        
                        [group[@"Attributes"] enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
                            
                            NSArray *idAndFormat = [key componentsSeparatedByString:@","];
                            
                            if (idAndFormat.count == 2) {
                                [attributes setObject:@{@"name": obj, @"format": idAndFormat[1]} forKey:idAndFormat[0]];
                            }
                            else {
                                [attributes setObject:obj forKey:key];
                            }
                        }];
                        
                        overrides = [attributes copy];
                        
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
                        
                        NSString *name = overriddenName ? overriddenName : [HWMATASmartInterfaceWrapper getDefaultAttributeNameByIdentifier:attribute->attributeId isRotational:_isRotational];
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
                        
                        if ([HWMEngine sharedEngine].configuration.notifyAlarmLevelChanges.boolValue && ATTRIBUTE_FLAGS_PREFAILURE(attribute->flag) && level == kHWMSensorLevelExceeded) {

                            [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                                        description:[NSString stringWithFormat:GetLocalizedString(@"'%@' S.M.A.R.T. attribute is critical for %@. Drive failure predicted!"), title, _product]
                                                   notificationName:NotifierSensorLevelExceededNotification
                                                           iconData:nil
                                                           priority:0
                                                           isSticky:YES
                                                       clickContext:nil];
                        }
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
    if (_smartInterface) {
        (*_smartInterface)->Release(_smartInterface);
    }
    
    if (_pluginInterface) {
        IODestroyPlugInInterface(_pluginInterface);
    }
}

-(void)dealloc
{
    //NSLog(@"Wrapper deallocated for %@", _bsdName);
    [self releaseInterface];
}

@end

