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

@implementation HWMThemedTextField

-(void)awakeFromNib
{
    HWMColorTheme *colorTheme = [HWMEngine sharedEngine].configuration.colorTheme;

    if (colorTheme) {
        [self setTextColor:colorTheme.itemTitleColor];
    }
}

@end
