//
//  HWMonitorSensorCell.h
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#include "ColorTheme.h"

@interface SensorCell : NSTableCellView

@property (assign) IBOutlet NSTextField *subtitleField;
@property (assign) IBOutlet NSTextField *valueField;
@property (assign) IBOutlet NSButton *checkBox;

@property (nonatomic, strong) ColorTheme *colorTheme;

//@property (nonatomic, strong) id representedObject;

@end
