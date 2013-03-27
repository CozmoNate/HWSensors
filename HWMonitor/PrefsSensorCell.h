//
//  ItemCell.h
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

@interface PrefsSensorCell : NSTableCellView

@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *valueField;
@property (nonatomic, unsafe_unretained) IBOutlet NSButton *checkBox;

@end
