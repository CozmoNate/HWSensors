//
//  PopupFanController.h
//  HWMonitor
//
//  Created by Kozlek on 08.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

@class HWMColorTheme, HWMSmcFanController;

@interface PopupFanController : NSViewController <NSTableViewDataSource, NSTableViewDelegate>
{
    NSArray *_levelsSnapshot;
}

@property (nonatomic, strong) HWMSmcFanController * controller;

@property (readonly) NSArray * inputSources;

//@property (assign) IBOutlet NSButton *controlledCheck;
//@property (assign) IBOutlet NSSlider *targetSlider;
//@property (assign) IBOutlet NSTextField *targetTextField;

@property (assign) IBOutlet NSTextField *inputLabel;
@property (assign) IBOutlet NSTextField *outputLabel;
@property (assign) IBOutlet NSPopUpButton *inputsPopUp;
@property (assign) IBOutlet NSTableView *levelsTableView;

@end
