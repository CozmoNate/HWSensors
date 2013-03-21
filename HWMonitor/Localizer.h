//
//  Localizer.h
//  HWMonitor
//
//  Created by kozlek on 20.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

#define GetLocalizedString(key) \
[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

@interface Localizer : NSObject

+ (void)localizeView:(id)view;

@end
