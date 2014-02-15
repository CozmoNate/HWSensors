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

static NSMutableDictionary * gIOCFPlugInInterfaces = nil;
static NSDictionary * gSmartAttributeOverridesDatabase = nil;
static NSMutableDictionary * gSmartAttributeOverrides = nil;

#define RAW_TO_LONG(attribute)  (UInt64)attribute->rawvalue[0] | \
(UInt64)attribute->rawvalue[1] << 8 | \
(UInt64)attribute->rawvalue[2] << 16 | \
(UInt64)attribute->rawvalue[3] << 24 | \
(UInt64)attribute->rawvalue[4] << 32 | \
(UInt64)attribute->rawvalue[5] << 40

#define GetLocalizedAttributeName(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:@"SmartAttributes"]

@implementation HWMSmartPluginInterfaceWrapper

@synthesize pluginInterface = _pluginInterface;
@synthesize smartInterface = _smartInterface;
@synthesize attributes = _attributes;

+(HWMSmartPluginInterfaceWrapper*)wrapperWithService:(io_service_t)service productName:(NSString*)productName firmware:(NSString*)firmware bsdName:(NSString*)bsdName isRotational:(BOOL)rotational
{

    if (!gIOCFPlugInInterfaces) {
        gIOCFPlugInInterfaces = [[NSMutableDictionary alloc] init];
    }

    HWMSmartPluginInterfaceWrapper *wrapper = [gIOCFPlugInInterfaces objectForKey:bsdName];

    if (!wrapper) {
        IOCFPlugInInterface ** pluginInterface = NULL;
        SInt32 score = 0;
        int tryCount = 4;

        do {

            if (S_OK == IOCreatePlugInInterfaceForService(service, kIOATASMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInterface, &score)) {

                (*pluginInterface)->AddRef(pluginInterface);

                IOATASMARTInterface ** smartInterface = NULL;

                if (S_OK == (*pluginInterface)->QueryInterface(pluginInterface, CFUUIDGetUUIDBytes(kIOATASMARTInterfaceID), (LPVOID)&smartInterface)) {

                    (*smartInterface)->AddRef(smartInterface);

                    wrapper = [[HWMSmartPluginInterfaceWrapper alloc] initWithPluginInterface:pluginInterface smartInterface:smartInterface productName:productName firmware:firmware bsdName:bsdName isRotational:rotational];

                    [gIOCFPlugInInterfaces setObject:wrapper forKey:bsdName];

                    break;
                }
                else if (smartInterface) {
                    (*pluginInterface)->Release(pluginInterface);
                    (*smartInterface)->Release(smartInterface);
                    IODestroyPlugInInterface(pluginInterface);

                    [NSThread sleepForTimeInterval:0.25];
                }
            }

        } while (--tryCount);

    }

    return wrapper;
}

+(HWMSmartPluginInterfaceWrapper*)getWrapperForBsdName:(NSString*)name
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


    if (!gSmartAttributeOverrides) {
        gSmartAttributeOverrides = [[NSMutableDictionary alloc] init];

        if (!gSmartAttributeOverridesDatabase) {
            if (!(gSmartAttributeOverridesDatabase = [NSDictionary dictionaryWithContentsOfURL:[[NSBundle mainBundle] URLForResource:@"smart-overrides" withExtension:@"plist"]])) {
                gSmartAttributeOverridesDatabase = [NSDictionary dictionary]; // Empty dictionary
            }
        }

    }

    NSString *key = [NSString stringWithFormat:@"%@%@", product, firmware];

    NSDictionary *overrides = [gSmartAttributeOverrides objectForKey:key];

    if (!overrides) {

        for (NSDictionary *group in gSmartAttributeOverridesDatabase.allValues) {

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

                        overrides = [group[@"Attributes"] copy];

                        [gSmartAttributeOverrides setObject:overrides forKey:key];
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
            (*_smartInterface)->SMARTEnableDisableAutosave(_smartInterface, true);
        }
    }

    if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTReturnStatus(_smartInterface, &exceeded))) {
        if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTReadData(_smartInterface, &smartData))) {
            if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTValidateReadData(_smartInterface, &smartData))) {

                bcopy(&smartData.vendorSpecific1, &_vendorSpecificData, sizeof(_vendorSpecificData));

                if (kIOReturnSuccess == (result = (*_smartInterface)->SMARTReadDataThresholds(_smartInterface, &smartDataThresholds))) {
                    bcopy(&smartDataThresholds.vendorSpecific1, &_vendorSpecificThresholds, sizeof(_vendorSpecificThresholds));
                }
                else {
                    NSLog(@"Failed to read S.M.A.R.T. thresholds");
                }

                // Prepare SMART attributes list
                if (!gSmartAttributeOverrides) {
                    gSmartAttributeOverrides = [[NSMutableDictionary alloc] init];
                }

                _overrides = [HWMSmartPluginInterfaceWrapper getAttributeOverridesForProduct:_product firmware:_firmware];

                NSMutableArray * attributes = [[NSMutableArray alloc] init];

                NSUInteger count = 0;

                for (int index = 0; index < kATASMARTAttributesCount; index++) {

                    ATASMARTAttribute *attribute = &_vendorSpecificData.vendorAttributes[index];

                    if (attribute->attributeId) {

                        ATASmartThresholdAttribute *threshold = &_vendorSpecificThresholds.ThresholdEntries[index];

                        NSString *overridden = _overrides ? [_overrides objectForKey:[NSString stringWithFormat:@"%d",attribute->attributeId]] : nil;

                        NSString *name = overridden ? overridden : [HWMSmartPluginInterfaceWrapper getDefaultAttributeNameByIdentifier:attribute->attributeId isRotational:_rotational];

                        NSString *title = GetLocalizedAttributeName(name);

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
                                                            description:[NSString stringWithFormat:GetLocalizedString(@"'%@' S.M.A.R.T. attribute is critical for %@. Drive failure predicted!"), title, _product]
                                                       notificationName:NotifierSensorLevelExceededNotification
                                                               iconData:nil
                                                               priority:0
                                                               isSticky:YES
                                                           clickContext:nil];
                                break;
                        }

                        [attributes addObject:@{@"index" : [NSNumber numberWithUnsignedInteger:count++],
                                                @"id": [NSNumber numberWithUnsignedChar:attribute->attributeId],
                                                @"name": name,
                                                @"title":titleColor ? [[NSAttributedString alloc] initWithString:title attributes:@{NSForegroundColorAttributeName:titleColor}] : title,
                                                @"critical": GetLocalizedString(critical ? @"Pre-Failure" : @"Life-Span"),
                                                @"value": [NSNumber numberWithUnsignedChar:attribute->current],
                                                @"worst": [NSNumber numberWithUnsignedChar:attribute->worst],
                                                @"threshold": (threshold ? [NSNumber numberWithUnsignedChar:threshold->ThresholdValue] : @0),
                                                @"raw": [NSNumber numberWithUnsignedLongLong:RAW_TO_LONG(attribute)]
                                               }];
                    }
                }

                _attributes = [attributes copy];
            }
        }
    }

    if (result != kIOReturnSuccess)
        NSLog(@"readSMARTData returned error for BSD device: %@", _bsdName);
    
    return result == kIOReturnSuccess;
}

-(HWMSmartPluginInterfaceWrapper*)initWithPluginInterface:(IOCFPlugInInterface**)pluginInterface smartInterface:(IOATASMARTInterface**)smartInterface productName:(NSString*)productName firmware:(NSString*)firmware bsdName:(NSString*)bsdName isRotational:(BOOL)rotational
{
    self = [super init];

    if (self) {
        _pluginInterface = pluginInterface;
        _smartInterface = smartInterface;
        _product = productName;
        _firmware = firmware;
        _bsdName = bsdName;
        _rotational = rotational;

        [self readSMARTDataAndThresholds];
    }

    return self;
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
@synthesize lastUpdated = _lastUpdated;

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

    gAvailableMountedPartitions = [partitions copy];
}

+(NSDictionary*)partitions
{
    if (!gAvailableMountedPartitions) {
        [HWMAtaSmartSensor updatePartitionsList];
    }

    return gAvailableMountedPartitions;
}

-(NSArray *)attributes
{
    if (!_attributes) {
        HWMSmartPluginInterfaceWrapper *wrapper = [HWMSmartPluginInterfaceWrapper getWrapperForBsdName:self.bsdName];

        if (!wrapper) {

            wrapper = [HWMSmartPluginInterfaceWrapper wrapperWithService:(io_service_t)self.service.unsignedLongLongValue
                                                             productName:self.productName
                                                                firmware:self.revision
                                                                 bsdName:self.bsdName
                                                            isRotational:self.rotational.boolValue];
        }

        _attributes = wrapper ? [wrapper.attributes copy] : @[];
        _lastUpdated = [NSDate date];
    }

    return _attributes;
}

-(void)awakeFromFetch
{
    [super awakeFromFetch];

    _temperatureAttributeIndex = -1;
    _remainingLifeAttributeIndex = -1;

    [self addObserver:self forKeyPath:@"self.engine.configuration.useBsdDriveNames" options:NSKeyValueObservingOptionNew context:nil];
    [self addObserver:self forKeyPath:@"self.engine.configuration.showVolumeNames" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];

    _temperatureAttributeIndex = -1;
    _remainingLifeAttributeIndex = -1;

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


-(BOOL)findIndexOfAttributeByName:(NSString*)name outIndex:(NSInteger*)index
{
    NSArray *results = [self.attributes filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name = %@", name]];

    *index = results && results.count ? [[results objectAtIndex:0][@"index"] unsignedIntegerValue] : -1;

    return *index > -1;
}

-(void)updateVolumeNames
{
    NSString *volumes = [[[HWMAtaSmartSensor partitions] objectForKey:self.bsdName] componentsJoinedByString:@", "];
    
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

    NSNumber *raw = [self.attributes objectAtIndex:_temperatureAttributeIndex][@"raw"];

    return [NSNumber numberWithUnsignedShort:raw.unsignedShortValue];
}

-(NSNumber*)getRemainingLife
{
    if (_remainingLifeAttributeIndex < 0) {

        if (![self findIndexOfAttributeByName:@"SSD_Life_Left" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Remaining_Lifetime_Perc" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Media_Wearout_Indicator" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Perc_Rated_Life_Used" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Wear_Leveling_Count" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Available_Reservd_Space" outIndex:&_remainingLifeAttributeIndex])
        {
            return nil;
        }
    }

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
    _attributes = nil;

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

static void block_device_appeared(void *engine, io_iterator_t iterator)
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{

        io_object_t object;

        __block NSMutableArray *devices = [NSMutableArray array];

        while ((object = IOIteratorNext(iterator))) {

            NSLog(@"ATA block storage device appeared %u", object);

            CFBooleanRef capable = (CFBooleanRef)IORegistryEntryCreateCFProperty(object, CFSTR(kIOPropertySMARTCapableKey), kCFAllocatorDefault, 0);

            if (capable != IO_OBJECT_NULL) {
                if (CFBooleanGetValue(capable)) {

                    NSDictionary * characteristics = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(object, CFSTR("Device Characteristics"), kCFAllocatorDefault, 0);

                    if (characteristics) {
                        NSString *name = [characteristics objectForKey:@"Product Name"];
                        NSString *serial = [characteristics objectForKey:@"Serial Number"];
                        NSString *medium = [characteristics objectForKey:@"Medium Type"];
                        NSString *revision = [characteristics objectForKey:@"Product Revision Level"];

                        if (name && serial && revision) {
                            NSString *volumes;
                            NSString *bsdName;

                            CFStringRef bsdNameRef = IORegistryEntrySearchCFProperty(object, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);

                            if (MACH_PORT_NULL != bsdNameRef) {
                                volumes = [[[HWMAtaSmartSensor partitions] objectForKey:(__bridge id)(bsdNameRef)] componentsJoinedByString:@", "];
                                bsdName = [(__bridge NSString*)bsdNameRef copy];
                                CFRelease(bsdNameRef);
                            }

                            if (bsdName) {
                                [devices addObject:@{@"service" : [NSNumber numberWithUnsignedLongLong:object],
                                                     @"productName": name,
                                                     @"bsdName" :bsdName,
                                                     @"volumesNames" : (volumes ? volumes : bsdName) ,
                                                     @"serialNumber" : serial,
                                                     @"revision" : revision,
                                                     @"rotational" : [NSNumber numberWithBool:medium ? ![medium isEqualToString:@"Solid State"] : TRUE]}
                                 ];
                            }
                        }
                    }
                }

                CFRelease(capable);
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
