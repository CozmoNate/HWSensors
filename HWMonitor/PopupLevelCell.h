//
//  PopupLevelCell.h
//  HWMonitor
//
//  Created by Kozlek on 02/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

@class HWMSmcFanControlLevel;

@interface PopupLevelCell : NSTableCellView

@property (assign) IBOutlet NSSlider *inputSlider;
@property (assign) IBOutlet NSTextField *inputTextField;
@property (assign) IBOutlet NSSlider *outputSlider;
@property (assign) IBOutlet NSTextField *outputTextField;

@property (readonly) HWMSmcFanControlLevel * level;

-(IBAction)sliderHasMoved:(id)sender;
-(IBAction)insertLevel:(id)sender;
-(IBAction)removeLevel:(id)sender;

@end
