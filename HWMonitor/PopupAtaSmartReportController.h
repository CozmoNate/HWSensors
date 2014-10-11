//
//  PopupAtaSmartReportControllerViewController.h
//  HWMonitor
//
//  Created by Kozlek on 07.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "HWMAtaSmartSensor.h"

@interface PopupAtaSmartReportController : NSViewController <NSTableViewDelegate>

@property (assign) IBOutlet NSArrayController * arrayController;
@property (assign) IBOutlet NSTableView * tableView;

@property (nonatomic, strong) HWMAtaSmartSensor * sensor;

- (IBAction)copy:(id)sender;

@end
