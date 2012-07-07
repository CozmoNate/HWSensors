//
//  HWMonitorGraphsView.h
//  HWSensors
//
//  Created by kozlek on 07.07.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//

@interface HWMonitorGraphsView : NSView
{
    NSRect _graphBounds;
    NSMutableDictionary *_graphs;
}

@property (readwrite, assign) NSUInteger group;

@property (assign) IBOutlet NSArrayController *content;

- (void)captureDataToHistoryFromDictionary:(NSDictionary*)info;

@end
