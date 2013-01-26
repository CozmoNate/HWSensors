//
//  GaphsTableDelegate.m
//  HWSensors
//
//  Created by kozlek on 24.01.13.
//  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#import "HWMonitorGraphsController.h"
#import "HWMonitorDefinitions.h"

@implementation HWMonitorGraphsController

@synthesize scrollView = _scrollView;
@synthesize tableView = _tableView;

- (void) setupController;
{
    [_tableView setShowsStickyRowHeader:YES];
}

- (void) awakeFromNib
{
	[self setupController];
}

- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    return nil != [[[self arrangedObjects] objectAtIndex:row] valueForKey:kHWMonitorKeySeparator];
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return nil == [[[self arrangedObjects] objectAtIndex:row] valueForKey:kHWMonitorKeySeparator];
}

- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if ([cell isKindOfClass:[NSButtonCell class]])
        [cell setTransparent:![cell isEnabled]];
//    else if ([cell isKindOfClass:[NSTextFieldCell class]] && nil != [[[self arrangedObjects] objectAtIndex:row] valueForKey:kHWMonitorKeySeparator])
//        [cell setAlignment:NSCenterTextAlignment];
}

@end
