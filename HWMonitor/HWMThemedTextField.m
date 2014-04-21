//
//  ThemedTextFieldView.m
//  HWMonitor
//
//  Created by Kozlek on 04.04.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "HWMThemedTextField.h"

#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"

#import <ReactiveCocoa/ReactiveCocoa.h>

@implementation HWMThemedTextField

-(void)awakeFromNib
{
    [RACObserve([HWMEngine sharedEngine].configuration, colorTheme) subscribeNext:^(HWMColorTheme *theme) {
        if (theme) {
            [self setTextColor:theme.itemTitleColor];
        }
    }];
}

@end
