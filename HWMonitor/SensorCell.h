//
//  HWMonitorSensorCell.h
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#include "ColorTheme.h"

@interface SensorCell : NSTableCellView

@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *subtitleField;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *valueField;
@property (nonatomic, unsafe_unretained) IBOutlet NSButton *checkBox;
@property (nonatomic, unsafe_unretained) IBOutlet NSColorWell *colorWell;

@property (nonatomic, strong) ColorTheme *colorTheme;

@property (nonatomic, strong) id representedObject;

@end
