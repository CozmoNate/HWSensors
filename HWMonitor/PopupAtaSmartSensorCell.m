//
//  ATASensorCell.m
//  HWMonitor
//
//  Created by Kozlek on 14/09/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupAtaSmartSensorCell.h"
#import "NSPopover+Message.h"
#import "HWMAtaSmartSensor.h"
#import "HWMColorTheme.h"
#import "HWMItem.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"

static NSMutableDictionary *smartctl_output = nil;
static NSMutableDictionary *smartctl_updated = nil;

@implementation PopupAtaSmartSensorCell

-(NSString *)smartOutput
{
    if (!smartctl_output) {
        smartctl_output = [[NSMutableDictionary alloc] init];
    }

    if (!smartctl_updated) {
        smartctl_updated = [[NSMutableDictionary alloc] init];
    }

    NSString *bsdName = [(HWMAtaSmartSensor*)self.objectValue bsdName];
    NSString *output = (NSString *)[smartctl_output objectForKey:bsdName];
    NSDate *updated = (NSDate *)[smartctl_updated objectForKey:bsdName];

    if (!output || !updated || [updated timeIntervalSinceNow] < -(60 * 10)) {
        NSTask *task = [[NSTask alloc] init];

        [task setLaunchPath: [[NSBundle mainBundle] pathForResource:@"smartctl" ofType:@""]];
        [task setArguments:[NSArray arrayWithObjects: @"-s", @"on", @"-A", bsdName, nil]];

        NSPipe *pipe = [NSPipe pipe];

        [task setStandardOutput: pipe];

        NSFileHandle *file = [pipe fileHandleForReading];

        [task launch];

        NSData *data = [file readDataToEndOfFile];

        output = CFBridgingRelease(CFStringCreateWithBytes(kCFAllocatorDefault, data.bytes, data.length, kCFStringEncodingUTF8, FALSE));

        updated = [NSDate dateWithTimeIntervalSinceNow:0.0];

        [smartctl_output setObject:output forKey:bsdName];
        [smartctl_updated setObject:updated forKey:bsdName];
    }

    return output;
}

- (void)showSmartOutput:(id)sender
{
    NSString *output = self.smartOutput;

    if (!_popover && output) {
        //[self.imageView.animator setAlphaValue:0.0];

        _popover = [NSPopover showRelativeToRect:self.frame
                                          ofView:self.superview
                                   preferredEdge:NSMinXEdge
                                          string:output
                                 backgroundColor:self.colorTheme.useDarkIcons.boolValue ?
                                                    [self.colorTheme.listBackgroundColor shadowWithLevel:0.2] :
                                                    [self.colorTheme.listBackgroundColor highlightWithLevel:0.2]
                                 foregroundColor:self.colorTheme.itemTitleColor
                                            font:[NSFont fontWithName:@"Monaco" size:10]
                                        maxWidth:750];
    }
}

-(void)resetCursorRects
{
    if (_trackingRectTag) {
        [self removeTrackingRect:_trackingRectTag];
    }

   _trackingRectTag = [self addTrackingRect:[self frame] owner:self userData:nil assumeInside:YES];
}

-(void)mouseEntered:(NSEvent *)theEvent
{
    [self performSelector:@selector(showSmartOutput:) withObject:self afterDelay:0.5];
}

-(void)mouseDown:(NSEvent *)theEvent
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [super mouseDown:theEvent];
}

-(void)mouseExited:(NSEvent *)theEvent
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if (_popover) {
        [_popover performClose:self];
        _popover = nil;
    }

    [super mouseExited:theEvent];
}

@end
