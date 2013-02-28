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


#import <Foundation/Foundation.h>

#import "HWMonitorSensor.h"
#import "ATASmartReporter.h"

@interface HWMonitorEngine : NSObject
{
@private
    io_connect_t _connection;
    NSATASmartReporter *_smartReporter;
    NSMutableArray *_sensors;
    NSMutableDictionary *_keys;
    NSLock *_sensorsLock;
}

@property (nonatomic, strong) NSBundle* bundle;

@property (readonly) NSArray *sensors;
@property (readonly) NSDictionary *keys;

@property (nonatomic, assign) BOOL useFahrenheit;
@property (nonatomic, setter = setUseBSDNames:) BOOL useBSDNames;

+ (HWMonitorEngine*)engineWithBundle:(NSBundle*)bundle;

+ (NSString*)copyTypeFromKeyInfo:(NSArray*)info;
+ (NSData*)copyValueFromKeyInfo:(NSArray*)info;

- (HWMonitorSensor*)addSensorWithKey:(NSString*)key title:(NSString*)title group:(NSUInteger)group;
- (HWMonitorSensor*)addSMARTSensorWithGenericDisk:(ATAGenericDisk*)disk group:(NSUInteger)group;

- (id)init;
- (id)initWithBundle:(NSBundle*)mainBundle;
- (void)dealloc;

- (NSArray*)populateInfoForKey:(NSString *)key;
- (void)rebuildSensorsList;
- (NSArray*)updateSmartSensors;
- (NSArray*)updateSmcSensors;
- (NSArray*)updateSmcSensorsList:(NSArray*)sensors;

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group;

@end
