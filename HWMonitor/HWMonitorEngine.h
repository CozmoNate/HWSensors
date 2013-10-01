//
//  HWMonitorEngine.h
//  HWSensors
//
//  Created by kozlek on 23/02/12.
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

#import "HWMonitorSensor.h"
#import "ATAGenericDrive.h"
#import "GenericBatteryDevice.h"

@interface HWMonitorEngine : NSObject
{
@private
    io_connect_t _connection;
    NSArray *_smartDrives;
    NSArray *_bluetoothDevices;
    NSMutableArray *_sensors;
    NSMutableDictionary *_keys;
    NSLock *_sensorsLock;
    NSArray *_currentProfile;
}

@property (nonatomic, strong) NSBundle* bundle;

@property (readonly) NSArray *sensors;
@property (readonly) NSDictionary *keys;

@property (nonatomic, setter = setUseFahrenheit:) BOOL useFahrenheit;
@property (nonatomic, setter = setUseBsdNames:) BOOL useBsdNames;

+ (HWMonitorEngine*)engineWithBundle:(NSBundle*)bundle;

- (HWMonitorSensor*)addSensorWithKey:(NSString*)key andTitle:(NSString*)title andGroup:(NSUInteger)group;
- (HWMonitorSensor*)addSmartSensorWithGenericDisk:(ATAGenericDrive*)disk group:(NSUInteger)group;
- (HWMonitorSensor*)addBluetoothSensorWithGenericDevice:(GenericBatteryDevice*)device group:(NSUInteger)group;

- (id)init;
- (id)initWithBundle:(NSBundle*)mainBundle;

- (NSData*)getSmcKeyInfoForKey:(NSString*)key;
- (void)rebuildInternalSensorsList;
- (NSArray*)updateSmartSensors;
- (NSArray*)updateSensors;
- (NSArray*)updateSensorsInArray:(NSArray*)sensors;

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group;

@end
