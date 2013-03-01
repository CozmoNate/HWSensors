//
//  HWMonitorGroupCell.h
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#include "ColorTheme.h"

@interface GroupCell : NSTableCellView
{
    NSGradient *_gradient;
    ColorTheme *_colorTheme;
}

@property (nonatomic, setter = setColorTheme:) ColorTheme *colorTheme;

@end
