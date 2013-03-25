//
//  GraphsSensorCell.h
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

@interface GraphsSensorCell : NSTableCellView

@property (nonatomic, unsafe_unretained) IBOutlet NSColorWell *colorWell;
@property (nonatomic, unsafe_unretained) IBOutlet NSButton *checkBox;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *valueField;

@end
