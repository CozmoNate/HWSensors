//
//  ATASensorCell.m
//  HWMonitor
//
//  Created by Kozlek on 14/09/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "ATASensorCell.h"
#import "NSPopover+Message.h"

@implementation ATASensorCell

- (void)mouseOverAction:(id)sender
{
    if (_cursorIsInsideTheFrame) {
        
        if (!_output) {
            NSTask *task = [[NSTask alloc] init];
            
            [task setLaunchPath: [[NSBundle mainBundle] pathForResource:@"smartctl" ofType:@""]];
            [task setArguments:[NSArray arrayWithObjects: @"-s", @"on", @"-A", _genericDrive.bsdName, nil]];
            
            NSPipe *pipe = [NSPipe pipe];
            
            [task setStandardOutput: pipe];
            
            NSFileHandle *file = [pipe fileHandleForReading];
            
            [task launch];
            
            NSData *data = [file readDataToEndOfFile];
            
            _output = CFBridgingRelease(CFStringCreateWithBytes(kCFAllocatorDefault, data.bytes, data.length, kCFStringEncodingUTF8, FALSE));
        }
        
        if (!_popover) {
            _popover = [NSPopover showRelativeToRect:[sender frame]
                                              ofView:[sender superview]
                                       preferredEdge:NSMinXEdge
                                              string:_output
                                     backgroundColor:[NSColor colorWithCalibratedWhite:0.95 alpha:0.95]
                                     foregroundColor:[NSColor blackColor]
                                                font:[NSFont fontWithName:@"Monaco" size:10]
                                            maxWidth:750];
        }
    }
}

-(void)resetCursorRects
{
    if (_trackingRectTag) {
        [self removeTrackingRect:_trackingRectTag];
    }
    
   _trackingRectTag = [self addTrackingRect:[self frame] owner:self userData:(__bridge void *)(_genericDrive) assumeInside:YES];
}

-(void)mouseEntered:(NSEvent *)theEvent
{
    _cursorIsInsideTheFrame = YES;
    
    [self performSelector:@selector(mouseOverAction:) withObject:self afterDelay:0.7];
}

-(void)mouseExited:(NSEvent *)theEvent
{
    _cursorIsInsideTheFrame = NO;
    
    [_popover performClose:self];
    _popover = nil;
}

@end
