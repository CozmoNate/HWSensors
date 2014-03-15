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

static NSPopover *gPopupSensorCellPopover;

@implementation PopupSensorCell

+(NSPopover*)globalPopover
{
    if (!gPopupSensorCellPopover) {
        gPopupSensorCellPopover = [[NSPopover alloc] init];
    }

    return gPopupSensorCellPopover;
}

+(void)destroyGlobalPopover
{
    if (gPopupSensorCellPopover) {
        if (gPopupSensorCellPopover.isShown) {
            [gPopupSensorCellPopover close];
        }
        gPopupSensorCellPopover = nil;
    }
}

+(void)setGlobalPopover:(NSPopover *)popover
{
    [PopupSensorCell destroyGlobalPopover];

    gPopupSensorCellPopover = popover;
}

-(void)initialize
{
    [super initialize];

    [self addObserver:self forKeyPath:@"objectValue.alarmLevel" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)deallocate
{
    [super deallocate];

    [self removeObserver:self forKeyPath:@"objectValue.alarmLevel"];
}

-(void)colorThemeChanged:(HWMColorTheme *)newColorTheme
{
    if ([self.objectValue alarmLevel] != kHWMSensorLevelExceeded) {
        [self.textField setTextColor:newColorTheme.itemTitleColor];
        [self.subtitleField setTextColor:newColorTheme.itemSubTitleColor];
    }

    if ([self.objectValue alarmLevel] == kHWMSensorLevelNormal) {
        [self.valueField setTextColor:newColorTheme.itemValueTitleColor];
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"objectValue.alarmLevel"]) {

        switch ([self.objectValue alarmLevel]) {
            case kHWMSensorLevelNormal:
                [self.valueField setTextColor:[self.objectValue engine].configuration.colorTheme.itemValueTitleColor];
                break;

            case kHWMSensorLevelModerate:
                [self.valueField setTextColor:[NSColor colorWithDeviceRed:0.7f green:0.3f blue:0.03f alpha:1.0f]];
                break;

            case kHWMSensorLevelHigh:
                [self.valueField setTextColor:[NSColor redColor]];
                break;

            case kHWMSensorLevelExceeded:
                [self.textField setTextColor:[NSColor redColor]];
                [self.subtitleField setTextColor:[NSColor redColor]];
                [self.valueField setTextColor:[NSColor redColor]];
                break;
        }

    }

    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

@end
