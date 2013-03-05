//
//  PopupView.h
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  Based on code by Vadim Shpanovski <https://github.com/shpakovski/Popup>
//  Popup is licensed under the BSD license.
//  Copyright (c) 2013 Vadim Shpanovski, Natan Zalkin. All rights reserved.
//

#define ARROW_WIDTH         23
#define ARROW_HEIGHT        11
#define ARROW_OFFSET        3
#define CORNER_RADIUS       6
#define LINE_THICKNESS      1

#import "ColorTheme.h"

@interface PopupView : NSView
{
    NSInteger       _arrowPosition;
    ColorTheme      *_colorTheme;
    NSRect          _contentRect;
    NSBezierPath    *_headerPath;
    NSGradient      *_headerGradient;
    NSBezierPath    *_contentPath;
}

@property (nonatomic, setter = setArrowPosition:) NSInteger arrowPosition;
@property (nonatomic, setter = setColorTheme:) ColorTheme *colorTheme;

@end
