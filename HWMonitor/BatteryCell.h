//
//  BatteryCell.h
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#include "ColorTheme.h"

@interface BatteryCell : NSTableCellView

@property (assign) IBOutlet NSTextField *subtitleField;
@property (assign) IBOutlet NSTextField *valueField;

@property (nonatomic, assign) NSUInteger gaugeLevel;
@property (nonatomic, strong) ColorTheme *colorTheme;

@end
