//
//  PopoverWindowFrameView.m
//  HWMonitor
//
//  Created by Kozlek on 7/6/15.
//  Copyright (c) 2015 kozlek. All rights reserved.
//

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

#import "PopoverWindowFrameView.h"
#import "PopoverWindow.h"

@implementation PopoverWindowFrameView

- (void)drawRect:(NSRect)rect
{
    PopoverWindow * window = (PopoverWindow*)self.window;
    [window drawRect:rect];
}

@end
