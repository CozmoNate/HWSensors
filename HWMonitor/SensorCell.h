//
//  HWMonitorSensorCell.h
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "ColorTheme.h"
#import "BasicCell.h"

@interface SensorCell : BasicCell

@property (assign) IBOutlet NSTextField *subtitleField;
@property (assign) IBOutlet NSTextField *valueField;
@property (assign) IBOutlet NSButton *checkBox;

@end
