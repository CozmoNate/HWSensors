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

#import "FakeSMCDefinitions.h"

@implementation HWMSmcFanSensor

@dynamic descriptor;
@dynamic number;
@dynamic max;
@dynamic min;
@dynamic speed;

-(void)awakeFromFetch
{
    [self addObserver:self forKeyPath:@"engine.configuration.enableFanControl" options:NSKeyValueObservingOptionNew context:nil];

    [super awakeFromFetch];
}

-(void)awakeFromInsert
{
    [self addObserver:self forKeyPath:@"engine.configuration.enableFanControl" options:NSKeyValueObservingOptionNew context:nil];

    [super awakeFromInsert];
}

-(void)prepareForDeletion
{
    [self removeObserver:self forKeyPath:@"engine.configuration.enableFanControl"];

    [super prepareForDeletion];
}

-(void)writeSMCKey:(NSString*)key value:(NSNumber*)value
{
    SMCVal_t info;

    if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.service.unsignedLongValue, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {
        if ([SmcHelper encodeNumericValue:value length:info.dataSize type:info.dataType outBuffer:info.bytes]) {
            SMCWriteKeyUnsafe((io_connect_t)self.service.unsignedLongValue, &info);
        }
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"engine.configuration.enableFanControl"]) {
        if (!self.engine.isRunningOnMac) {

            SMCVal_t info;

            if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.service.unsignedLongValue, KEY_FAN_MANUAL, &info)) {

                NSNumber *value;

                if ((value = [SmcHelper decodeNumericValueFromBuffer:&info.bytes length:info.dataSize type:info.dataType])) {

                    UInt16 manual = value.unsignedShortValue;

                    bit_write(self.engine.configuration.enableFanControl.boolValue, manual, BIT(self.number.unsignedShortValue));

                    [self writeSMCKey:@KEY_FAN_MANUAL value:[NSNumber numberWithUnsignedShort:manual]];
                }
            }
        }
    }
    
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

-(void)setSpeed:(NSNumber *)speed
{
    if (self.engine.configuration.enableFanControl.boolValue && self.max && self.min && self.number && speed) {

        if (self.engine.isRunningOnMac) {
            [self writeSMCKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_TARGET, self.number.unsignedCharValue] value:speed];
        }
        else {
            [self writeSMCKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_TARGET, self.number.unsignedCharValue] value:speed];
        }

        [self willChangeValueForKey:@"speed"];
        [self setPrimitiveValue:speed forKey:@"speed"];
        [self didChangeValueForKey:@"speed"];
    }
}

@end
