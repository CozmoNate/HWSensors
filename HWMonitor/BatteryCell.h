//
//  BatteryCell.h
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#include "ColorTheme.h"

@interface BatteryCell : NSTableCellView

@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *subtitleField;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *valueField;
@property (nonatomic, assign) NSUInteger gaugeLevel;

@property (nonatomic, strong) ColorTheme *colorTheme;

@end
