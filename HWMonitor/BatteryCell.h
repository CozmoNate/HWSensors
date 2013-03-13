//
//  BatteryCell.h
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#include "ColorTheme.h"

@interface BatteryCell : NSTableCellView

@property (nonatomic, unsafe_unretained) IBOutlet NSImageView *mouseImageView;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *mouseTextField;
@property (nonatomic, unsafe_unretained) IBOutlet NSImageView *keyboardImageView;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *keyboardTextField;

@end
