//
//  JLNAutoSizingTokenField.m
//  JLNAutoSizingTokenField
//
//  Created by Joshua Nozzi on 9/17/10.
//  Copyright (c) 2010 Joshua Nozzi. All rights reserved.
//
//	This source code is free for any personal and commercial uses.
//	This source code comes with no warranty. Use at your own risk.
//
//	Instructional blog post:
//	http://joshua.nozzi.name/2010/06/non-selecting-token-field/
//


#import "JLNAutoSizingTokenField.h"


@implementation JLNAutoSizingTokenField


- (id)init
{
    if (self = [super init])
        needsSizeToFit = NO;
    return self;
}


#pragma mark Auto-Size-To-Fit

- (BOOL)needsSizeToFit
{
	return needsSizeToFit;
}

- (void)setNeedsSizeToFit:(BOOL)flag
{
	needsSizeToFit = flag;
	if (flag)
		[self performSelector:@selector(sizeToFitIfNeeded) 
				   withObject:nil 
				   afterDelay:0];
}

- (void)sizeToFitIfNeeded
{
	if (needsSizeToFit)
		[self sizeToFit];
	needsSizeToFit = NO;
}

- (void)textDidEndEditing:(NSNotification *)aNotification
{
	[self setNeedsSizeToFit:YES];
	[super textDidEndEditing:aNotification];
}


@end
