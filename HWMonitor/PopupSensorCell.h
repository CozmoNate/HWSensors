//
//  HWMonitorSensorCell.h
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "PopupBasicCell.h"

@interface PopupSensorCell : PopupBasicCell

@property (assign) IBOutlet NSTextField *subtitleField;
@property (assign) IBOutlet NSTextField *valueField;
@property (assign) IBOutlet NSButton *checkBox;

@end
