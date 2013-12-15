//
//  HWMSmcFanSensor.m
//  HWMonitor
//
//  Created by Kozlek on 22/11/13.
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

#import "HWMSmcFanSensor.h"
#import "smc.h"
#import "SmcHelper.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"

@implementation HWMSmcFanSensor

@dynamic number;
@dynamic max;
@dynamic min;
@dynamic speed;
@dynamic unique;

-(void)setSpeed:(NSNumber *)speed
{
    if (self.engine.configuration.enableFanControl.boolValue && self.max && self.min && self.number && self.speed && speed) {
        SMCVal_t info;
        
        char key[5];
        
        //snprintf(str, 5, "F%dMn", self.number.unsignedCharValue);
        snprintf(key, 5, "F%XTg", self.number.unsignedCharValue);
        
        if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.service.unsignedLongValue, key, &info)) {
            if ([SmcHelper encodeNumericValue:speed.floatValue length:info.dataSize type:info.dataType outBuffer:info.bytes]) {
                SMCWriteKeyUnsafe((io_connect_t)self.service.unsignedLongValue, &info);
            }
        }
    }
    
    [self willChangeValueForKey:@"speed"];
    [self setPrimitiveValue:speed forKey:@"speed"];
    [self didChangeValueForKey:@"speed"];
}

@end
