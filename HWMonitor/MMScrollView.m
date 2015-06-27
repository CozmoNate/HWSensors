//
//  MMScrollView.m
//  MiniMail
//
//  Created by DINH Viêt Hoà on 24/08/10.
//  Copyright 2011 Sparrow SAS. All rights reserved.
//

#import "MMScrollView.h"

@interface MMScrollView()

@property (retain) NSScroller *retainVerticalScroller;
    
@end

@implementation MMScrollView

@synthesize retainVerticalScroller = _retainVerticalScroller;

- (void)tile
{
	NSRect frame;
	CGFloat height;
    
	[[self contentView] setFrame:[self bounds]];
    height = [self bounds].size.height;
	frame = NSMakeRect([self bounds].size.width - 15, 0, 15, height);
	[[self verticalScroller] setFrame:frame];
	
    if ([self.documentView frame].size.height <= self.frame.size.height) {
        self.retainVerticalScroller = self.verticalScroller;
        [self.verticalScroller removeFromSuperview];
    } else {
        NSScroller * verticalScroller = self.verticalScroller;
        [verticalScroller removeFromSuperview];
        [self addSubview:verticalScroller];
    }
}

@end
