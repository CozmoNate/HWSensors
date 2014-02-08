//
//  HWMonitorSensorCell.h
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "PopupBasicCell.h"

@interface PopupSensorCell : PopupBasicCell

+(NSPopover*)globalPopover;
+(void)destroyGlobalPopover;
+(void)setGlobalPopover:(NSPopover*)popover;

@property (assign) IBOutlet NSTextField *subtitleField;
@property (assign) IBOutlet NSTextField *valueField;

@end
