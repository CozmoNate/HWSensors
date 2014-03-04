//
//  PopupLevelCell.m
//  HWMonitor
//
//  Created by Kozlek on 02/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopupLevelCell.h"
#import "HWMSmcFanController.h"
#import "HWMSmcFanControlLevel.h"

#define ROUND50(x) (((int)x / 50) * 50)
#define CLIP(x) (x) < 0 ? 0 : (x) > 100 ? 100 : (x)

@implementation PopupLevelCell

-(HWMSmcFanControlLevel *)level
{
    return self.objectValue;
}

- (void)sliderHasMoved:(id)sender
{
    NSTextField *textField = nil;
    NSUInteger value = 0;

    if (sender == self.inputSlider) {
        textField = self.inputTextField;
        value = [sender integerValue];
        value = value < self.level.minInput.integerValue ? self.level.minInput.integerValue : value > self.level.maxInput.integerValue ? self.level.maxInput.integerValue : value;
    }
    else if (sender == self.outputSlider) {
        textField = self.outputTextField;
        value = [sender integerValue];
        value = value < self.level.minOutput.integerValue ? self.level.minOutput.integerValue : value > self.level.maxOutput.integerValue ? self.level.maxOutput.integerValue : value;
        value = ROUND50(value);
    }

    [textField setIntegerValue:value];
    [sender setIntegerValue:value];

    SEL sel = @selector(sliderHasBeenReleased:);

    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:sel object:sender];

    [self performSelector:sel withObject:sender afterDelay:0.0];
}

- (void)sliderHasBeenReleased:(id)sender
{
    if (sender == self.inputSlider) {
        [self.level setInput:[NSNumber numberWithInteger:[sender integerValue]]];
    }
    else if (sender == self.outputSlider) {
        [self.level setOutput:[NSNumber numberWithInteger:ROUND50([sender integerValue])]];
    }
}

-(IBAction)insertLevel:(id)sender
{
    [self.level insertNextLevel];
}

-(IBAction)removeLevel:(id)sender
{
    [self.level removeThisLevel];
}

@end
