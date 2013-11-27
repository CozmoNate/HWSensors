//
//  HWMonitorSensorCell.m
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "PopupSensorCell.h"
#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensor.h"
#import "HWMSensorsGroup.h"

@implementation PopupSensorCell

-(void)colorThemeHasChanged:(HWMColorTheme *)newColorTheme
{
    [self.textField setTextColor:newColorTheme.itemTitleColor];
    [self.subtitleField setTextColor:newColorTheme.itemSubTitleColor];
    [self.valueField setTextColor:newColorTheme.itemValueTitleColor];
}

@end
