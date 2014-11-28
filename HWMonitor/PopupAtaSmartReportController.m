//
//  PopupAtaSmartReportControllerViewController.m
//  HWMonitor
//
//  Created by Kozlek on 07.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopupAtaSmartReportController.h"

#import "NSPopover+Message.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"

#import "Localizer.h"

#import "NSWindow+BackgroundBlur.h"

@interface PopupAtaSmartReportController ()

@end

@implementation PopupAtaSmartReportController

-(void)setSensor:(HWMAtaSmartSensor *)sensor
{
    _sensor = sensor;


}

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    if (self) {

    }

    return self;
}

-(void)awakeFromNib
{
    [Localizer localizeView:_tableView];
    [_tableView setGridColor:[HWMEngine sharedEngine].configuration.colorTheme.useBrightIcons.boolValue ? [NSColor colorWithCalibratedWhite:0.4 alpha:1.0] : [NSColor colorWithCalibratedWhite:0.8 alpha:1.0]];
}

- (void)copy:(id)sender;
{
    NSArray	*selectedObjects = [self.arrayController selectedObjects];

    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];

	[pasteboard clearContents];

    NSMutableString *entry = [[NSMutableString alloc] init];

    // Copy column names
    [_tableView.tableColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSCell *headerCell = [obj headerCell];

        if (headerCell.title) {
            [entry appendString:headerCell.title];

            if (idx < _tableView.tableColumns.count - 1) {
                [entry appendString:@", "];
            }
        }
    }];

    if (![pasteboard writeObjects:@[entry]]) {
        NSBeep();
        return;
    }

    // Copy attributes
	for (NSDictionary *item in selectedObjects ) {

        entry = [[NSString stringWithFormat:@"%d, %@, %@, %d, %d, %d, %@",
                           [item[@"id"] unsignedCharValue],
                           item[@"name"],
                           item[@"critical"],
                           [item[@"value"] unsignedShortValue],
                           [item[@"worst"] unsignedShortValue],
                           [item[@"threshold"] unsignedShortValue],
                           item[@"raw"]] mutableCopy];

		if (![pasteboard writeObjects:@[entry]]) {
            NSBeep();
            break;
        }
    }
}

@end
