//
//  HWMonitorSensor.h
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

#include "ATAGenericDrive.h"

typedef enum {
    kHWSensorGroupNone                  = 0,
    
    kHWSensorGroupTemperature           = 1 << 1,
    kHWSensorGroupVoltage               = 1 << 2,
    kHWSensorGroupPWM                   = 1 << 3,
    kHWSensorGroupTachometer            = 1 << 4,
    kHWSensorGroupMultiplier            = 1 << 5,
    kHWSensorGroupFrequency             = 1 << 6,
    kHWSensorGroupCurrent              = 1 << 7,
    kHWSensorGroupPower                 = 1 << 8,
    
    kSMARTGroupTemperature        = 1 << 9,
    kSMARTGroupRemainingLife      = 1 << 10,
    kSMARTGroupRemainingBlocks    = 1 << 11,
} HWSensorGroup;

typedef enum {
    kHWSensorLevelUnused                = 0,
    kHWSensorLevelDisabled              = 1,
    kHWSensorLevelNormal                = 2,
    kHWSensorLevelModerate              = 3,
    kHWSensorLevelHigh                  = 4,
    kHWSensorLevelExceeded              = 1000,
} HWSensorLevel;

@class HWMonitorEngine;

@interface HWMonitorSensor : NSObject
{
    NSNumber *_rawValue;
}

@property (nonatomic, strong) HWMonitorEngine*  engine;

@property (nonatomic, strong) NSString* name;
@property (nonatomic, strong) NSString* type;
@property (nonatomic, assign) NSUInteger group;
@property (nonatomic, strong) NSString* title;
@property (nonatomic, strong) NSData* data;
@property (nonatomic, strong) id genericDevice;

@property (nonatomic, assign) HWSensorLevel level;
@property (readonly) NSNumber* rawValue;
@property (readonly) NSString* formattedValue;

@property (readonly) BOOL levelHasBeenChanged;
@property (readonly) BOOL valueHasBeenChanged;

@property (nonatomic, strong) id representedObject;

+ (int)getIndexOfHexChar:(char)c;
+ (HWMonitorSensor*)sensor;

- (void)setType:(NSString *)newType;
- (NSString *)type;
- (void)setData:(NSData *)newData;
- (NSData *)data;

- (float)decodeNumericValue;
- (NSString*)formattedValue;

@end
