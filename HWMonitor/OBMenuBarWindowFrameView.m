//
//  RoundWindowFrameView.m
//  RoundWindow
//
//  Created by Matt Gallagher on 12/12/08.
//  Copyright 2008 Matt Gallagher. All rights reserved.
//
//  Permission is given to use this source code file without charge in any
//  project, commercial or otherwise, entirely at your risk, with the condition
//  that any redistribution (in part or whole) of source code must retain
//  this copyright and permission notice. Attribution in compiled projects is
//  appreciated but not required.
//

#import "OBMenuBarWindowFrameView.h"
#import "OBMenuBarWindow.h"

@implementation OBMenuBarWindowFrameView

//
// drawRect:
//
// Draws the frame of the window.
//
- (void)drawRect:(NSRect)rect
{
    OBMenuBarWindow * window = (OBMenuBarWindow*)self.window;
	[window drawRect:rect];
}

@end
