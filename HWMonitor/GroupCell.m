//
//  HWMonitorGroupCell.m
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "GroupCell.h"
#import "PopupView.h"

@implementation GroupCell

-(void)setColorTheme:(ColorTheme *)colorTheme
{
    _colorTheme = colorTheme;
    
    _gradient = [[NSGradient alloc]
                 initWithStartingColor:_colorTheme.groupBackgroundStartColor
                 endingColor:_colorTheme.groupBackgroundEndColor];
    
    [self.textField setTextColor:_colorTheme.groupTitleColor];
    
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (!_gradient) {
        _gradient = [[NSGradient alloc]
                    initWithStartingColor:_colorTheme.groupBackgroundStartColor
                              endingColor:_colorTheme.groupBackgroundEndColor];
    }
    
    NSRect contentRect = [self bounds];
    
    [_gradient drawInRect:NSMakeRect(contentRect.origin.x + LINE_THICKNESS, contentRect.origin.y, contentRect.size.width - LINE_THICKNESS * 2, contentRect.size.height) angle:270];
}

@end
