//
//  GraphsSensorCell.h
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

@class GraphsColorWell;

@interface GraphsSensorCell : NSTableCellView

@property (assign) IBOutlet GraphsColorWell *colorWell;
@property (assign) IBOutlet NSButton *checkBox;
@property (assign) IBOutlet NSTextField *valueField;

@end
