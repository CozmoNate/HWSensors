//
//  NoodleRowSpanningTableView.m
//  NoodleRowSpanningTableViewTest
//
//  Created by Paul Kim on 10/20/09.
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

#import "NoodleTableView.h"
#import "NSTableView-NoodleExtensions.h"
#import "NSImage-NoodleExtensions.h"
#import "NSIndexSet-NoodleExtensions.h"

/*
 Internal cell class. Wraps around another cell. Draws the inner cell in its "full frame" but when drawing in
 the tableview, draws each row sliver from the full image.
 */
@interface NoodleRowSpanningCell : NSCell
{
	NSRect			_fullFrame;
	NSCell			*_cell;
	NSImage			*_cachedImage;
	NSColor			*_backgroundColor;
	NSInteger		_startIndex;
	NSInteger		_lastStartIndex;
	NSInteger		_endIndex;
	NSInteger		_lastEndIndex;
}

@property NSRect fullFrame;
@property (assign) NSCell *cell;
@property (copy) NSColor *backgroundColor;
@property NSInteger startIndex;
@property NSInteger endIndex;

@end

@implementation NoodleRowSpanningCell

@synthesize fullFrame = _fullFrame;
@synthesize cell = _cell;
@synthesize backgroundColor = _backgroundColor;
@synthesize startIndex = _startIndex;
@synthesize endIndex = _endIndex;

- (void)_clearOutCaches
{
	_startIndex = -1;
	_endIndex = -1;
	_lastStartIndex = -1;
	_lastEndIndex = -1;
	_cell = nil;
}

- (void)dealloc
{
	[self _clearOutCaches];
	[_backgroundColor release];
	[_cachedImage release];
	
	[super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
	NoodleRowSpanningCell		*copy;
 
	copy = [super copyWithZone:zone];
 
	// super will copy the pointer across (via NSCopyObject()) but we need to retain or copy the actual instances
	copy->_cachedImage = [_cachedImage copy];
	copy->_backgroundColor = [_backgroundColor copy];
 
	return copy;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	// Draw the full span of the cell into a cached image and then pull out the correct sliver as needed

	if ((_startIndex != _lastStartIndex) || (_endIndex != _lastEndIndex) || (_cachedImage == nil))
	{
		// If the indices have changed, we are dealing with a new span so recache the cell's full image
		NSAffineTransform	*transform;
		NSColor				*color;
		
		if ((_cachedImage == nil) || !NSEqualSizes(_fullFrame.size, [_cachedImage size]))
		{
			[_cachedImage release];
			_cachedImage = [[NSImage alloc] initWithSize:_fullFrame.size];
			[_cachedImage setFlipped:[controlView isFlipped]];
		}
		
		[_cachedImage lockFocus];
		
		transform = [NSAffineTransform transform];
		[transform translateXBy:-NSMinX(_fullFrame) yBy:-NSMinY(_fullFrame)];
		[transform concat];

		color = _backgroundColor;
		if (color == nil)
		{
			color = [NSColor clearColor];
		}
		[color set];
		NSRectFill(_fullFrame);
 
		[_cell drawWithFrame:_fullFrame inView:controlView];

		[_cachedImage unlockFocus];
		
		_lastStartIndex = _startIndex;
		_lastEndIndex = _endIndex;
	}
	
	// Now draw the sliver for the current row in the right spot
	[_cachedImage drawAdjustedInRect:cellFrame
							fromRect:NSMakeRect(NSMinX(cellFrame) - NSMinX(_fullFrame),
												NSMinY(cellFrame) - NSMinY(_fullFrame), 
												NSWidth(cellFrame), NSHeight(cellFrame))
						   operation:NSCompositeSourceOver fraction:1.0];
}

@end

@implementation NoodleTableColumn : NSTableColumn

@synthesize rowSpanningEnabled = _spanningEnabled;

#define SPANNING_ENABLED_KEY			@"spanningEnabled"

- (void)encodeWithCoder:(NSCoder *)encoder
{
	[super encodeWithCoder:encoder];
	
	if ([encoder allowsKeyedCoding])
	{
		[encoder encodeObject:[NSNumber numberWithBool:_spanningEnabled] forKey:SPANNING_ENABLED_KEY];
	}
	else
	{
		[encoder encodeObject:[NSNumber numberWithBool:_spanningEnabled]];
	}
}

- (id)initWithCoder:(NSCoder *)decoder
{
	if ((self = [super initWithCoder:decoder]) != nil)
	{
		id			value;
		
		if ([decoder allowsKeyedCoding])
		{
			value = [decoder decodeObjectForKey:SPANNING_ENABLED_KEY];
		}
		else
		{
			value = [decoder decodeObject];
		}
	
		if (value != nil)
		{
			_spanningEnabled = [value boolValue];
		}
	}
	return self;
}

- (void)dealloc
{
	[_cell release];
	[super dealloc];
}

- (NoodleRowSpanningCell *)spanningCell
{
	if (_cell == nil)
	{
		_cell = [[NoodleRowSpanningCell alloc] initTextCell:@""];
	}
	return _cell;
}

@end


@implementation NoodleTableView

@synthesize showsStickyRowHeader = _showsStickyRowHeader;

#pragma mark NSCoding methods

#define SHOWS_STICKY_ROW_HEADER_KEY			@"showsStickyRowHeader"

- (void)encodeWithCoder:(NSCoder *)encoder
{
	[super encodeWithCoder:encoder];
	
	if ([encoder allowsKeyedCoding])
	{
		[encoder encodeObject:[NSNumber numberWithBool:_showsStickyRowHeader] forKey:SHOWS_STICKY_ROW_HEADER_KEY];
	}
	else
	{
		[encoder encodeObject:[NSNumber numberWithBool:_showsStickyRowHeader]];
	}
}

- (id)initWithCoder:(NSCoder *)decoder
{
	if ((self = [super initWithCoder:decoder]) != nil)
	{
		id			value;
		
		if ([decoder allowsKeyedCoding])
		{
			value = [decoder decodeObjectForKey:SHOWS_STICKY_ROW_HEADER_KEY];
		}
		else
		{
			value = [decoder decodeObject];
		}
		_showsStickyRowHeader = [value boolValue];
		
		for (NSTableColumn *column in [self tableColumns])
		{
			if ([column isKindOfClass:[NoodleTableColumn class]])
			{
				_hasSpanningColumns = YES;
				break;
			}
		}
	}
	return self;
}

- (void)addTableColumn:(NSTableColumn *)column
{
	[super addTableColumn:column];
	
	if ([column isKindOfClass:[NoodleTableColumn class]])
	{
		_hasSpanningColumns = YES;
	}
}

- (void)removeTableColumn:(NSTableColumn *)column
{
	[super removeTableColumn:column];
	
	for (/*NSTableColumn **/column in [self tableColumns])
	{
		if ([column isKindOfClass:[NoodleTableColumn class]])
		{
			_hasSpanningColumns = YES;
			break;
		}
	}
}

#pragma mark Row Spanning methods

- (void)setRowSpanningEnabledForCapableColumns:(BOOL)flag
{
	for (id column in [self tableColumns])
	{
		if ([column respondsToSelector:@selector(setRowSpanningEnabled:)])
		{
			[column setRowSpanningEnabled:flag];
		}
	}
}

// Does the actual work of drawing the grid. Originally, was trying to set the grid mask and calling super's
// -drawGridInClipRect: method on specific regions to get the effect I wanted but all the setting of the masks
// ended up sucking down CPU cycles as it got into a loop queueing up tons of redraw requests.
- (void)_drawGrid:(NSUInteger)gridMask inClipRect:(NSRect)aRect
{
	NSRect			rect;
	
	[[self gridColor] set];
	
	if ((gridMask & NSTableViewSolidHorizontalGridLineMask) != 0)
	{
		NSRange			range;
		NSUInteger		i;

		range = [self rowsInRect:aRect];
		for (i = range.location; i < NSMaxRange(range); i++)
		{
			rect = [self rectOfRow:i];
			if (NSMaxY(rect) <= NSMaxY(aRect))
			{
				rect.origin.x = NSMinX(aRect);
				rect.size.width = NSWidth(aRect);
				rect.origin.y -= 0.5;
				[NSBezierPath strokeLineFromPoint:NSMakePoint(NSMinX(rect), NSMaxY(rect)) toPoint:NSMakePoint(NSMaxX(rect), NSMaxY(rect))];
			}
		}
	}
	if ((gridMask & NSTableViewSolidVerticalGridLineMask) != 0)
	{
		NoodleIndexSetEnumerator	*enumerator;
		NSInteger					i;
		
		enumerator = [[self columnIndexesInRect:aRect] indexEnumerator];
		while ((i = [enumerator nextIndex]) != NSNotFound)
		{
			rect = [self rectOfColumn:i];
			if (NSMaxX(rect) <= NSMaxX(aRect))
			{
				rect.origin.y = NSMinY(aRect);
				rect.size.height = NSHeight(aRect);
				rect.origin.x -= 0.5;
				[NSBezierPath strokeLineFromPoint:NSMakePoint(NSMaxX(rect), NSMinY(rect)) toPoint:NSMakePoint(NSMaxX(rect), NSMaxY(rect))];
			}
		}
	}
}

- (void)drawGridInClipRect:(NSRect)aRect
{
	NSUInteger					origGridMask;
		
	origGridMask = [self gridStyleMask];
	
	if (_hasSpanningColumns && ((origGridMask & NSTableViewSolidHorizontalGridLineMask) != 0))
	{
		// Rules:
		// - No spanning cell should have grid lines within it. Only at the bottom.
		// - Non-spanning cells inherit their grid lines from the closest spanning cell to the left (or to the right
		// if there are none to the left.
		
		NSRange						range, spanRange;
		NSUInteger					columnIndex, endColumnIndex, startColumnIndex, row;
		NSMutableIndexSet			*columnIndexes;
		NoodleIndexSetEnumerator	*enumerator;
		NSRect						topLeft, bottomRight, rect;
		
		// Grab the indexes of all the spanning columns.
		columnIndex = 0;
		columnIndexes = [NSMutableIndexSet indexSet];
		for (NSTableColumn *column in [self tableColumns])
		{
			if ([column isRowSpanningEnabled])
			{
				[columnIndexes addIndex:columnIndex];
			}
			columnIndex++;
		}
		
		// Hard to explain but we calculate regions going from left to right, defining regions horizontally
		// from one spanning column to the next and vertically by row spans. We first draw the non-horizontal
		// grid lines within a span (taking into account precedence rules concerning columns as noted above).
		// Then we draw the horizontal grid line at the bottom of a span. We are trying to find the maximal
		// regions to send to the grid drawing routine as doing it cell by cell incurs a bit of overhead.
		startColumnIndex = 0;
		enumerator = [columnIndexes indexEnumerator];
		while ((columnIndex = [enumerator nextIndex]) != NSNotFound)
		{
			// This column is the right edge of this region (which is up to the next spanning column)
			endColumnIndex = [columnIndexes indexGreaterThanIndex:columnIndex];
			if (endColumnIndex == NSNotFound)
			{
				endColumnIndex = [self numberOfColumns] - 1;
			}
			else
			{
				endColumnIndex--;
			}
			
			range = [self rowsInRect:aRect];
			
			row = range.location;
			while (row < NSMaxRange(range))
			{
				spanRange = [self rangeOfRowSpanAtColumn:columnIndex row:row];
				
				// Get the rects of the top left of our region (the start column and start row of the span)
				// to the bottom right (the end column and the row in the span just before the last one).
				topLeft = [self frameOfCellAtColumn:startColumnIndex row:spanRange.location];
				bottomRight = [self frameOfCellAtColumn:endColumnIndex row:NSMaxRange(spanRange) - 2];
				
				rect = NSIntersectionRect(aRect, NSUnionRect(topLeft, bottomRight));
				
				if (spanRange.length > 1)
				{
					// Draw span region without horizontal grid lines
					[self _drawGrid:origGridMask & ~NSTableViewSolidHorizontalGridLineMask inClipRect:rect];
					
					// Now, calculate the region at the last row of the span
					topLeft = [self frameOfCellAtColumn:startColumnIndex row:NSMaxRange(spanRange) - 1];
					bottomRight = [self frameOfCellAtColumn:endColumnIndex row:NSMaxRange(spanRange) - 1];
					
					// Draw bottom of span with horizontal grid lines
					rect = NSIntersectionRect(aRect, NSUnionRect(topLeft, bottomRight) );
					[self _drawGrid:origGridMask inClipRect:rect];
				}
				else
				{
					// Not a span row or just a single row. Either way, draw with horizontal grid lines
					[self _drawGrid:origGridMask inClipRect:rect];
				}
				// Advance to the next row span
				row = NSMaxRange(spanRange);
			}
			// Advance to the next span column region
			startColumnIndex = endColumnIndex + 1;
		}
	}
	else
	{
		// We only need the special logic when we have row spanning columns and drawing horizontal lines. Otherwise,
		// just call super.
		[super drawGridInClipRect:aRect];
	}
}

- (NSCell *)preparedCellAtColumn:(NSInteger)columnIndex row:(NSInteger)rowIndex
{
	NSTableColumn				*column;
	
	column = [[self tableColumns] objectAtIndex:columnIndex];	

	if (!_isDrawingStickyRow && [column isRowSpanningEnabled])
	{
		NSRange		range;
	
		range = [self rangeOfRowSpanAtColumn:columnIndex row:rowIndex];
		
		if (range.length >= 1)
		{
			// Here is where we insert our special cell for row spanning behavior
			NoodleRowSpanningCell	*spanningCell;
			NSCell					*cell;
			NSInteger				start, end;
			BOOL					wasSelected;

			start = range.location;
			end = NSMaxRange(range) - 1;
			
			// Want to draw cell in its unhighlighted state since spanning cells aren't selectable. Unfortuantely,
			// can't just setHighlight:NO on it because NSTableView sets other attributes (like text color).
			// Instead, we deselect the row, grab the cell, then set it back (if it was selected before).
			wasSelected = [self isRowSelected:start];
			[self deselectRow:start];
			
			cell = [super preparedCellAtColumn:columnIndex row:start];
			
			if (wasSelected)
			{
				[self selectRowIndexes:[NSIndexSet indexSetWithIndex:start] byExtendingSelection:YES];
			}
			
			spanningCell = [column spanningCell];
			[spanningCell setCell:cell];

			// The full frame is the rect encompassing the first and last rows of the span.
			[spanningCell setFullFrame:NSUnionRect([self frameOfCellAtColumn:columnIndex row:start],
												   [self frameOfCellAtColumn:columnIndex row:end])];
			[spanningCell setStartIndex:start];
			[spanningCell setEndIndex:end];				
			
			[spanningCell setBackgroundColor:[self backgroundColor]];

			return spanningCell;
		}
	}
	return [super preparedCellAtColumn:columnIndex row:rowIndex];
}

- (void)mouseDown:(NSEvent *)event
{
	if (_hasSpanningColumns)
	{
		// Eat up any clicks on spanning cells. In the future, may want to consider having clicks select the
		// first row in the span (like when clicking on the artwork in iTunes).

		NSPoint				point;
		NSInteger			columnIndex;
		NSTableColumn		*column;
		
		point = [event locationInWindow];
		point = [self convertPointFromBase:point];
		
		columnIndex = [self columnAtPoint:point];
		column = [[self tableColumns] objectAtIndex:columnIndex];
		if ([column isRowSpanningEnabled])
		{
			return;
		}
	}
	[super mouseDown:event];
}


- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	// All that needs to be done to enable the sticky row header functionality (bulk of the work
	// done in the NSTableView category)
	if ([self showsStickyRowHeader])
	{
		[self drawStickyRowHeader];
	}

	if (_hasSpanningColumns)
	{
		// Clean up any cached data. Don't want to keep stale cache data around.
		for (NSTableColumn *column in [self tableColumns])
		{
			if ([column isRowSpanningEnabled])
			{
				[[column spanningCell] _clearOutCaches];
			}
		}
	}
}

- (void)drawStickyRow:(NSInteger)row clipRect:(NSRect)clipRect
{
	_isDrawingStickyRow = YES;
	[super drawStickyRow:row clipRect:clipRect];
	_isDrawingStickyRow = NO;
}

@end
