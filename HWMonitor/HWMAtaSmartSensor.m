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

#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensorsGroup.h"

#import "HWMonitorDefinitions.h"
#import "Localizer.h"

#import <sys/mount.h>
#import <Growl/Growl.h>

static NSMutableDictionary * gIOCFPlugInInterfaces;

@implementation HWMSmartPlugInInterfaceWrapper

+(HWMSmartPlugInInterfaceWrapper*)wrapperWithService:(io_service_t)service forBsdName:(NSString*)name
{

    if (!gIOCFPlugInInterfaces) {
        gIOCFPlugInInterfaces = [[NSMutableDictionary alloc] init];
    }

    HWMSmartPlugInInterfaceWrapper *wrapper = [gIOCFPlugInInterfaces objectForKey:name];

    if (!wrapper) {
        IOCFPlugInInterface ** pluginInterface = NULL;
        SInt32 score = 0;

        wrapper = [[HWMSmartPlugInInterfaceWrapper alloc] init];

        if (S_OK == IOCreatePlugInInterfaceForService(service, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score)) {

            (*pluginInterface)->AddRef(pluginInterface);
            [wrapper setPluginInterface:pluginInterface];

            IOATASMARTInterface ** smartInterface = NULL;

            if (S_OK == (*wrapper.pluginInterface)->QueryInterface(wrapper.pluginInterface, CFUUIDGetUUIDBytes(kIOATASMARTInterfaceID), (LPVOID)&smartInterface)) {

                [wrapper setSmartInterface:smartInterface];

                [gIOCFPlugInInterfaces setObject:wrapper forKey:name];
            }
            else {
                if (smartInterface) {
                    (*pluginInterface)->Release(pluginInterface);
                    (*smartInterface)->Release(smartInterface);
                    IODestroyPlugInInterface(pluginInterface);
                    wrapper = nil;
                }
            }
        }
        else {
            wrapper = nil;
        }
    }

    return wrapper;
}

+(HWMSmartPlugInInterfaceWrapper*)getWrapperForBsdName:(NSString*)name
{
    if (name && gIOCFPlugInInterfaces) {
        return [gIOCFPlugInInterfaces objectForKey:name];
    }

    return nil;
}

+(void)destroyAllWrappers
{
    if (gIOCFPlugInInterfaces) {
        [gIOCFPlugInInterfaces enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
            [obj destroy];
        }];

        [gIOCFPlugInInterfaces removeAllObjects];
        gIOCFPlugInInterfaces = nil;
    }
}

-(void)destroy
{
    if (self.smartInterface) {
        (*self.smartInterface)->Release(self.smartInterface);
    }

    if (self.pluginInterface) {
        (*self.pluginInterface)->Release(self.pluginInterface);

        IODestroyPlugInInterface(self.pluginInterface);
    }
}

@end

@implementation HWMAtaSmartSensor

@dynamic bsdName;
@dynamic productName;
@dynamic volumeNames;
@dynamic serialNumber;
@dynamic rotational;

@synthesize exceeded = _exceeded;
@synthesize attributes = _attributes;

#define RAW_TO_LONG(attribute)  (UInt64)attribute->rawvalue[0] | \
(UInt64)attribute->rawvalue[1] << 8 | \
(UInt64)attribute->rawvalue[2] << 16 | \
(UInt64)attribute->rawvalue[3] << 24 | \
(UInt64)attribute->rawvalue[4] << 32 | \
(UInt64)attribute->rawvalue[5] << 40

static NSDictionary * gAttributeOverridesDatabase = nil;

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

+(NSDictionary*)getAttributeOverridesForProduct:(NSString*)product firmware:(NSString*)firmware
{
    if (!product)
        return nil;

    if (!gAttributeOverridesDatabase) {
        if (!(gAttributeOverridesDatabase = [NSDictionary dictionaryWithContentsOfURL:[[NSBundle mainBundle] URLForResource:@"smart-overrides" withExtension:@"plist"]])) {
            gAttributeOverridesDatabase = [NSDictionary dictionary]; // Empty dictionary
        }
    }

    for (NSDictionary *group in gAttributeOverridesDatabase.allValues) {

        NSArray *productMatch = group[@"NameMatch"];

        if (productMatch) {

            for (NSString *pattern in productMatch) {

                NSRegularExpression *expression = [NSRegularExpression regularExpressionWithPattern:pattern options:NSRegularExpressionCaseInsensitive error:nil];

                if ([expression numberOfMatchesInString:product options:NSMatchingReportCompletion range:NSMakeRange(0, product.length)]) {

                    NSArray *firmwareMatch = group[@"FirmwareMatch"];

                    if (firmware && firmwareMatch) {

                        BOOL supported = NO;

                        for (pattern in productMatch) {
                            expression = [NSRegularExpression regularExpressionWithPattern:pattern options:NSRegularExpressionCaseInsensitive error:nil];

                            if ([expression numberOfMatchesInString:product options:NSMatchingReportCompletion range:NSMakeRange(0, product.length)]) {
                                supported = YES;
                                break;
                            }
                        }

                        if (!supported) {
                            return nil;
                        }
                    }

                    return group[@"Attributes"];
                }
            }
        }
    }

    return nil;
}

+(NSArray*)discoverDrives
{
    NSMutableDictionary *partitions = [[NSMutableDictionary alloc] init];

	BOOL first = YES;

    NSArray *mountedVolumeURLs = [[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:[NSArray array] options:NSVolumeEnumerationSkipHiddenVolumes];

    for (NSURL *url in mountedVolumeURLs) {
        NSString *path = url.path;
        struct statfs buffer;

        if (statfs([path fileSystemRepresentation],&buffer) == 0)
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

    NSMutableArray * list = [[NSMutableArray alloc] init];

    CFDictionaryRef matching = IOServiceMatching("IOBlockStorageDevice");
    io_iterator_t iterator = IO_OBJECT_NULL;

    if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
        if (IO_OBJECT_NULL != iterator) {

            io_service_t service = MACH_PORT_NULL;

            while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {

                CFBooleanRef capable = (CFBooleanRef)IORegistryEntryCreateCFProperty(service, CFSTR(kIOPropertySMARTCapableKey), kCFAllocatorDefault, 0);

                if (capable != IO_OBJECT_NULL) {
                    if (CFBooleanGetValue(capable)) {

                        NSDictionary * characteristics = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(service, CFSTR("Device Characteristics"), kCFAllocatorDefault, 0);

                        if (characteristics) {
                            NSString *name = [characteristics objectForKey:@"Product Name"];
                            NSString *serial = [characteristics objectForKey:@"Serial Number"];
                            NSString *medium = [characteristics objectForKey:@"Medium Type"];
                            //                            NSString *revision = [characteristics objectForKey:@"Product Revision Level"];

                            if (name && serial) {
                                NSString *volumes;
                                NSString *bsdName;

                                CFStringRef bsdNameRef = IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);

                                if (MACH_PORT_NULL != bsdNameRef) {
                                    volumes = [[partitions objectForKey:(__bridge id)(bsdNameRef)] componentsJoinedByString:@", "];
                                    bsdName = [(__bridge NSString*)bsdNameRef copy];
                                    CFRelease(bsdNameRef);
                                }

                                if (bsdName) {

                                    if ([HWMSmartPlugInInterfaceWrapper wrapperWithService:service forBsdName:bsdName]) {

                                        [list addObject:@{@"service" : [NSNumber numberWithUnsignedLongLong:service],
                                                          @"productName": name,
                                                          @"bsdName" :bsdName,
                                                          @"volumesNames" : (volumes ? volumes : bsdName) ,
                                                          @"serialNumber" : serial,
                                                          @"rotational" : [NSNumber numberWithBool:medium ? ![medium isEqualToString:@"Solid State"] : TRUE]}
                                         ];
                                    }
                                }
                            }
                        }
                    }

                    CFRelease(capable);
                }
            }

            IOObjectRelease(iterator);
        }
    }

    [list sortUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        NSString *name1 = [(NSDictionary*)obj1 objectForKey:@"bsdName"];
        NSString *name2 = [(NSDictionary*)obj2 objectForKey:@"bsdName"];

        return [name1 compare:name2];
    }];

    return list;
}

-(NSArray *)attributes
{
    if (!_attributes) {

        if (!_overrides) {
            _overrides = [HWMAtaSmartSensor getAttributeOverridesForProduct:self.productName firmware:self.name];

            if (!_overrides) {
                _overrides = [NSDictionary dictionary];
            }
        }

        NSMutableArray * attributes = [[NSMutableArray alloc] init];

        for (int index = 0; index < kATASMARTAttributesCount; index++) {
            if (_smartData.vendorAttributes[index].attributeId) {

                ATASMARTAttribute *attribute = &_smartData.vendorAttributes[index];
                ATASmartThresholdAttribute *threshold = &_smartDataThresholds.ThresholdEntries[index];

                NSString *overridden = _overrides ? [_overrides objectForKey:[NSString stringWithFormat:@"%d",_smartData.vendorAttributes[index].attributeId]] : nil;

                NSString *name = overridden ? overridden : [HWMAtaSmartSensor getDefaultAttributeNameByIdentifier:attribute->attributeId isRotational:self.rotational.boolValue];
                NSString *title = GetLocalizedString(name);

                BOOL critical = ATTRIBUTE_FLAGS_PREFAILURE(attribute->flag);

                NSUInteger level = kHWMSensorLevelNormal;

                if (threshold)
                {
                    if (critical && attribute->current <= threshold->ThresholdValue) {
                        level = kHWMSensorLevelExceeded;
                    }
                }

                NSColor *titleColor = nil;

                switch (level) {
                    case kHWMSensorLevelExceeded:

                        titleColor = [NSColor redColor];

                        [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                                    description:[NSString stringWithFormat:GetLocalizedString(@"%@ S.M.A.R.T. attribute is critical: '%@'. Drive failure predicted!"), self.title, title]
                                               notificationName:NotifierSensorLevelExceededNotification
                                                       iconData:nil
                                                       priority:0
                                                       isSticky:YES
                                                   clickContext:nil];
                        break;
                }

                if (level == kHWMSensorLevelExceeded) {

                }

                [attributes addObject:@{@"id": [NSNumber numberWithUnsignedChar:attribute->attributeId],
                                        @"name": name,
                                        @"title":titleColor ? [[NSAttributedString alloc] initWithString:title attributes:@{NSForegroundColorAttributeName:titleColor}] : title,
                                        @"critical": GetLocalizedString(critical ? @"Pre-Failure" : @"Life-Span"),
                                        @"value": [NSNumber numberWithUnsignedChar:attribute->current],
                                        @"worst": [NSNumber numberWithUnsignedChar:attribute->worst],
                                        @"threshold": (threshold ? [NSNumber numberWithUnsignedChar:threshold->ThresholdValue] : @0),
                                        @"raw": [NSString stringWithFormat:@"(%1$llu)", RAW_TO_LONG(attribute)]}];
            }
        }

        _attributes = [attributes copy];
    }

    return _attributes;
}

-(void)awakeFromFetch
{
    [super awakeFromFetch];

    [self addObserver:self forKeyPath:@"self.engine.configuration.useBsdDriveNames" options:NSKeyValueObservingOptionNew context:nil];
    [self addObserver:self forKeyPath:@"self.engine.configuration.showVolumeNames" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];

    [self addObserver:self forKeyPath:@"self.engine.configuration.useBsdDriveNames" options:NSKeyValueObservingOptionNew context:nil];
    [self addObserver:self forKeyPath:@"self.engine.configuration.showVolumeNames" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)prepareForDeletion
{
    [super prepareForDeletion];

    [self removeObserver:self forKeyPath:@"self.engine.configuration.useBsdDriveNames"];
    [self removeObserver:self forKeyPath:@"self.engine.configuration.showVolumeNames"];

    IOObjectRelease((io_service_t)self.service.unsignedLongLongValue);
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"self.engine.configuration.useBsdDriveNames"]) {
        [self setTitle:self.engine.configuration.useBsdDriveNames.boolValue ? self.bsdName : self.productName];
    }
    else if ([keyPath isEqualToString:@"self.engine.configuration.showVolumeNames"]) {
        [self setLegend:self.engine.configuration.showVolumeNames.boolValue ? self.volumeNames : nil];
    }

    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

-(BOOL)readSMARTData
{
    if (updated && [updated timeIntervalSinceNow] > -60.0)
        return NO;

    IOReturn result = kIOReturnError;

    HWMSmartPlugInInterfaceWrapper *wrapper = [HWMSmartPlugInInterfaceWrapper getWrapperForBsdName:self.bsdName];

    if (wrapper) {

        ATASMARTData smartData;
        ATASMARTDataThresholds smartDataThresholds;

        bzero(&smartData, sizeof(smartData));
        bzero(&smartDataThresholds, sizeof(smartDataThresholds));

        _exceeded = false;

        if (kIOReturnSuccess != (*wrapper.smartInterface)->SMARTReturnStatus(wrapper.smartInterface, &_exceeded)) {
            if (kIOReturnSuccess != (*wrapper.smartInterface)->SMARTEnableDisableOperations(wrapper.smartInterface, true)) {
                (*wrapper.smartInterface)->SMARTEnableDisableAutosave(wrapper.smartInterface, true);
            }
        }

        if (kIOReturnSuccess == (result = (*wrapper.smartInterface)->SMARTReturnStatus(wrapper.smartInterface, &_exceeded))) {
            if (kIOReturnSuccess == (result = (*wrapper.smartInterface)->SMARTReadData(wrapper.smartInterface, &smartData))) {
                if (kIOReturnSuccess == (result = (*wrapper.smartInterface)->SMARTValidateReadData(wrapper.smartInterface, &smartData))) {
                    bcopy(&smartData.vendorSpecific1, &_smartData, sizeof(_smartData));

                    if (kIOReturnSuccess == (result = (*wrapper.smartInterface)->SMARTReadDataThresholds(wrapper.smartInterface, &smartDataThresholds))) {
                        bcopy(&smartDataThresholds.vendorSpecific1, &_smartDataThresholds, sizeof(_smartDataThresholds));
                    }
                    else {
                        NSLog(@"Failed to read S.M.A.R.T. thresholds");
                    }

                    // Release old attributes dictionary forcing it to rebuild with new data
                    _attributes = nil;

                    updated = [NSDate date];
                }
            }
        }
    }

    if (result != kIOReturnSuccess)
        NSLog(@"readSMARTData returned error for device %@", self.bsdName);

    return result == kIOReturnSuccess;
}

-(NSUInteger)indexOfAttributeByIdentifier:(UInt8)identifier
{
    for (NSUInteger index = 0; index < kATASMARTAttributesCount; index++)
        if (_smartData.vendorAttributes[index].attributeId == identifier)
            return index;

    return 0;
}

-(NSUInteger)indexOfAttributeByName:(NSString*)name
{
    NSArray *results = [self.attributes filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name = %@", name]];

    NSString *identifier = results && results.count ? [(NSDictionary*)[results objectAtIndex:0] objectForKey:@"id"] : nil;

    return identifier ? [self indexOfAttributeByIdentifier:identifier.integerValue] : 0;
}

-(NSNumber*)getTemperature
{
    if ([self readSMARTData] && !_temperatureAttributeIndex) {
        NSUInteger index = 0;

        if ((index = [self indexOfAttributeByName:@"Temperature_Celsius"]) ||
            (index = [self indexOfAttributeByName:@"Airflow_Temperature_Cel"]) ||
            (index = [self indexOfAttributeByName:@"Temperature_Case"])
            ) {
            _temperatureAttributeIndex = index;
        }
        else {
            return nil;
        }
    }

    ATASMARTAttribute *temperature = &_smartData.vendorAttributes[_temperatureAttributeIndex];

    NSUInteger value = temperature->rawvalue[0] && temperature->rawvalue[0] < 100 ? temperature->rawvalue[0] : temperature->current < 100 ? temperature->current : 0;

    return value ? [NSNumber numberWithUnsignedInteger:value] : nil;
}

-(NSNumber*)getRemainingLife
{
    if ([self readSMARTData] && !_remainingLifeAttributeIndex) {
        NSUInteger index = 0;

        if ((index = [self indexOfAttributeByName:@"SSD_Life_Left"]) ||
            (index = [self indexOfAttributeByName:@"Remaining_Lifetime_Perc"]) ||
            (index = [self indexOfAttributeByName:@"Media_Wearout_Indicator"]) ||
            (index = [self indexOfAttributeByName:@"Perc_Rated_Life_Used"]) ||
            (index = [self indexOfAttributeByName:@"Wear_Leveling_Count"]) ||
            (index = [self indexOfAttributeByName:@"Available_Reservd_Space"])
            ) {
            _remainingLifeAttributeIndex = index;
        }
        else {
            return nil;
        }
    }

    ATASMARTAttribute *life = &_smartData.vendorAttributes[_remainingLifeAttributeIndex];

    return [NSNumber numberWithUnsignedChar:life->current];
}

-(NSNumber*)getRemainingBlocks
{
    if ([self readSMARTData] && !_unusedBlocksAttributeIndex) {
        NSUInteger index = 0;

        if ((index = [self indexOfAttributeByName:@"Unused_Rsvd_Blk_Cnt_Tot"]) ) {
            _unusedBlocksAttributeIndex = index;
        }
        else {
            return nil;
        }
    }

    ATASMARTAttribute *blocks = &_smartData.vendorAttributes[_unusedBlocksAttributeIndex];

    return [NSNumber numberWithUnsignedChar:RAW_TO_LONG(blocks)];
}

-(NSUInteger)internalUpdateAlarmLevel
{
    float floatValue = self.value.floatValue;

    switch (self.selector.unsignedIntegerValue) {
        case kHWMGroupTemperature:
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
    switch (self.selector.unsignedIntegerValue) {
        case kHWMGroupTemperature:
        case kHWMGroupSmartTemperature:
            return [self getTemperature];

        case kHWMGroupSmartRemainingLife:
            return [self getRemainingLife];

        case kHWMGroupSmartRemainingBlocks:
            return [self getRemainingBlocks];
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
