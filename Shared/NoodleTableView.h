//
//  NoodleRowSpanningTableView.h
//  NoodleRowSpanningTableViewTest
//
//  Created by Paul Kim on 10/20/09.
//  Copyright 2009-2012 Noodlesoft, LLC. All rights reserved.
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

#import <Cocoa/Cocoa.h>

/*
 This NSTableView subclass provides a couple of neat features.
 
 Sticky Row Headers
 ================== 
 This allows you to specify certain rows (it uses group rows by default) to "stick" to the top of the tableview
 scroll area when it is scrolled out of view. This is similar to how section headers work in UITableView in the
 iPhone SDK. The bulk of the implementation and delegate API are in the NSTableView-NoodleExtensions category.
 
 To enable this feature, just set the showsStickyRowHeader property.
 
 For more details, see the related blog post at http://www.noodlesoft.com/blog/2009/09/25/sticky-section-headers-in-nstableview/

 
 Row Spanning Columns
 ====================
 
 Row Spanning Columns are columns whose cells are allow to span across multiple rows. The span is determined by
 a contiguous section of rows that have the same object value. The cells within such a span are consolidated into
 a single special cell. An example of this can be seen in the Artwork column in iTunes.
 
 For any columns where you want to have this take effect, just change the column class to NoodleTableColumn and
 set the rowSpanningEnabled property on it. You can alternatively call -setRowSpanningEnabledForCapablyColumns
 on the tableview in your -awakeFromNib to enable all the NoodleTableColumns in your table in one fell swoop.
 
 For more details, see the related blog post at http://www.noodlesoft.com/blog/2009/10/20/yet-another-way-to-mimic-the-artwork-column-in-cocoa/
 */
@interface NoodleTableView : NSTableView
{
	BOOL		_showsStickyRowHeader;
	
	BOOL		_hasSpanningColumns;
	BOOL		_isDrawingStickyRow;
}

@property (readwrite, assign) BOOL showsStickyRowHeader;

#pragma mark Row Spanning methods

/*
 Enables/disables row spanning for all NoodleTableColumns in the receiver. Note that row spanning is not enabled
 by default so if you want row spanning, call this from -awakeFromNib is a good idea.
 */
- (void)setRowSpanningEnabledForCapableColumns:(BOOL)flag;

@end


@class NoodleRowSpanningCell;

/*
 Special table column that enables row spanning functionality. Just set your columns in IB to use this class and
 enable it by calling -setRowSpaningEnabled:
 */
@interface NoodleTableColumn :NSTableColumn
{
	BOOL						_spanningEnabled;
	NoodleRowSpanningCell		*_cell;

}

@property (getter=isRowSpanningEnabled, setter=setRowSpanningEnabled:) BOOL rowSpanningEnabled;

@end
