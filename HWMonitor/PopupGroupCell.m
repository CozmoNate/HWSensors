//
//  HWMonitorGroupCell.m
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "PopupGroupCell.h"
#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensorsGroup.h"

@implementation PopupGroupCell

- (void)drawRect:(NSRect)dirtyRect
{
    if (!_gradient) {
        _gradient = [[NSGradient alloc] initWithStartingColor:self.colorTheme.groupStartColor
                                                  endingColor:self.colorTheme.groupEndColor];
    }
    
    NSRect contentRect = [self bounds];
    
    [_gradient drawInRect:NSMakeRect(contentRect.origin.x + 1.0, contentRect.origin.y, contentRect.size.width - 2.0, contentRect.size.height) angle:270];
}

-(void)colorThemeChanged:(HWMColorTheme *)newColorTheme
{
    _gradient = nil;
    [self setNeedsDisplay:YES];
}

@end
