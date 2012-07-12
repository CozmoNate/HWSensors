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
    double _minY;
    double _maxY;
    NSDictionary *_legendAttributes;
    NSString *_legendFormat;
}

@property (assign) IBOutlet NSArrayController *graphs;
@property (readwrite, assign) NSUInteger group;
@property (readwrite, assign) BOOL useFahrenheit;

- (void)captureDataToHistoryFromDictionary:(NSDictionary*)info;

@end
