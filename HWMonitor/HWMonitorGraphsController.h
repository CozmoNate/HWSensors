//
//  GaphsTableDelegate.h
//  HWSensors
//
//  Created by Natan Zalkin on 24.01.13.
//
//

#import "NoodleTableView.h"

@interface HWMonitorGraphsController : NSArrayController <NSTableViewDelegate>

@property (assign) IBOutlet NSScrollView *scrollView;
@property (assign) IBOutlet NoodleTableView *tableView;

- (void) setupController;

@end
