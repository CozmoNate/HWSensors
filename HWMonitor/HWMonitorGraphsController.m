//
//  GaphsTableDelegate.m
//  HWSensors
//
//  Created by Natan Zalkin on 24.01.13.
//
//

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
