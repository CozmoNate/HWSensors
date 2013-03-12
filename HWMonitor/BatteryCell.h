//
//  BatteryCell.h
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface BatteryCell : NSTableCellView

@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *mousePercentageField;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *keyboardPercentageField;

@end
