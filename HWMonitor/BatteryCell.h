//
//  BatteryCell.h
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#include "SensorCell.h"

@interface BatteryCell : SensorCell

@property (nonatomic, strong) NSNumber *gaugeLevel;

@end
