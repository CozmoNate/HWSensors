//
//  HWMAtaSmartSensor.h
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

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMSensor.h"

typedef enum : NSUInteger {
    kHWMDriveNameProduct,
    kHWMDriveNameVolumes,
    kHWMDriveNameBSD,
} HWMDriveNameSelector;

@interface HWMAtaSmartSensor : HWMSensor
{
    NSInteger _temperatureAttributeIndex;
    NSInteger _remainingLifeAttributeIndex;
}

@property (nonatomic, retain) NSString * bsdName;
@property (nonatomic, retain) NSString * productName;
@property (nonatomic, retain) NSString * volumeNames;
@property (nonatomic, retain) NSString * revision;
@property (nonatomic, retain) NSString * serialNumber;
@property (nonatomic, retain) NSNumber * rotational;

@property (readonly) NSArray *attributes;
@property (readonly) NSDictionary *overrides;

+(void)updatePartitionsList;

+(void)startWatchingForBlockStorageDevicesWithEngine:(HWMEngine*)engine;
+(void)stopWatchingForBlockStorageDevices;

-(void)updateVolumeNames;

@end

NSString* bsdname_from_service(io_registry_entry_t object);
