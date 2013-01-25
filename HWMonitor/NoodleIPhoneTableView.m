//
//  NoodleIPhoneTableView.m
//  NoodleKit
//
//  Created by Paul Kim on 9/22/09.
//  Copyright 2009 Noodlesoft, LLC. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#import "NoodleIPhoneTableView.h"
#import "NSTableView-NoodleExtensions.h"

@implementation NoodleIPhoneTableView

- (id)initWithFrame:(NSRect)frameRect
{
	if ((self = [super initWithFrame:frameRect]) != nil)
	{
		[self setGridColor:[NSColor colorWithCalibratedWhite:0.849 alpha:1.0]];
		[self setGridStyleMask:NSTableViewSolidHorizontalGridLineMask];
	}
	return self;
}

- (id)initWithCoder:(NSCoder *)decoder
{
	if ((self = [super initWithCoder:decoder]) != nil)
	{
		[self setGridColor:[NSColor colorWithCalibratedWhite:0.849 alpha:1.0]];
		[self setGridStyleMask:NSTableViewSolidHorizontalGridLineMask];
	}
	return self;
}

- (void)drawRect:(NSRect)rect
{
	[super drawRect:rect];
	
	[self drawStickyRowHeader];
}

// Since we are going to ensure that the regular and sticky versions of a row
// look the same, no transition is needed here.
- (NoodleStickyRowTransition)stickyRowHeaderTransition
{
	return NoodleStickyRowTransitionNone;
}

- (void)drawRow:(NSInteger)rowIndex clipRect:(NSRect)clipRect
{
	if ([self isRowSticky:rowIndex])
	{
		NSRect					rowRect, cellRect;
		NSUInteger				colIndex, count;
		NSCell					*cell;
		NSGradient				*gradient;
		NSAttributedString		*attrString;
		NSShadow				*textShadow;
		NSBezierPath			*path;
		NSDictionary			*attributes;
		
		rowRect = [self rectOfRow:rowIndex];

		if (!_isDrawingStickyRow)
		{
			// Note that NSTableView will still draw the special background that it does
			// for group row so we re-draw the background over it.
			[self drawBackgroundInClipRect:rowRect];
			
			if (NSIntersectsRect(rowRect, [self stickyRowHeaderRect]))
			{
				// You can barely notice it but if the sticky view is showing, the actual
				// row it represents is still seen underneath. We check for this and don't
				// draw the row in such a case.
				return;
			}
		}

		gradient = [[NSGradient alloc] initWithStartingColor:
					[NSColor colorWithCalibratedRed:0.490 green:0.556 blue:0.600 alpha:0.9]
					endingColor:[NSColor colorWithCalibratedRed:0.665 green:0.706 blue:0.738 alpha:0.9]];
		
		[gradient drawInRect:rowRect angle:90];
		[gradient release];

		textShadow = [[NSShadow alloc] init];
		[textShadow setShadowOffset:NSMakeSize(1.0, -1.0)];
		[textShadow setShadowColor:[NSColor colorWithCalibratedWhite:0.5 alpha:1.0]];
		[textShadow setShadowBlurRadius:0.0];
		
		attributes = [NSDictionary dictionaryWithObjectsAndKeys:
													 [NSFont fontWithName:@"Helvetica-Bold" size:16.0],
													 NSFontAttributeName,
													 textShadow,
													 NSShadowAttributeName,
													 [NSColor whiteColor],
													 NSForegroundColorAttributeName,
													 nil];
		[textShadow release];
		
		count = [self numberOfColumns];
		for (colIndex = 0; colIndex < count; colIndex++)
		{
			cell = [self preparedCellAtColumn:colIndex row:rowIndex];

			attrString = [[NSAttributedString alloc] initWithString:[cell stringValue] attributes:attributes];
			
			cellRect = [self frameOfCellAtColumn:colIndex row:rowIndex];
			
			[cell setAttributedStringValue:attrString];
			[cell drawWithFrame:cellRect inView:self];
			
			[attrString release];
		}
		
		[[NSColor colorWithCalibratedWhite:0.5 alpha:1.0] set];
		path = [NSBezierPath bezierPath];
		[path moveToPoint:NSMakePoint(NSMinX(rowRect), NSMinY(rowRect))];
		[path lineToPoint:NSMakePoint(NSMaxX(rowRect), NSMinY(rowRect))];
		[path moveToPoint:NSMakePoint(NSMinX(rowRect), NSMaxY(rowRect))];
		[path lineToPoint:NSMakePoint(NSMaxX(rowRect), NSMaxY(rowRect))];
		[path stroke];
	}
	else
	{
		[super drawRow:rowIndex clipRect:clipRect];
	}
}

- (void)drawStickyRow:(NSInteger)row clipRect:(NSRect)clipRect
{
	_isDrawingStickyRow = YES;
	[self drawRow:row clipRect:clipRect];
	_isDrawingStickyRow = NO;
}


@end
