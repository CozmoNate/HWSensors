//
//  JLNAutoSizingTokenField.h
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


#import <Cocoa/Cocoa.h>


@interface JLNAutoSizingTokenField : NSTokenField
{
    BOOL						needsSizeToFit;
}


#pragma mark Auto-Size-To-Fit

- (BOOL)needsSizeToFit;
- (void)setNeedsSizeToFit:(BOOL)flag;
- (void)sizeToFitIfNeeded;


@end

