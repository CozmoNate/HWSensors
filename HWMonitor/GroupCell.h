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
}

@property (nonatomic, strong) ColorTheme *colorTheme;

@end
