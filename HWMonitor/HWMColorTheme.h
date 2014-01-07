//
//  HWMColorTheme.h
//  HWMonitor
//
//  Created by Kozlek on 21/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMConfiguration;

@interface HWMColorTheme : NSManagedObject

@property (nonatomic, retain) HWMConfiguration * configuration;
@property (nonatomic, retain) NSString * name;
@property (nonatomic, retain) NSColor* groupEndColor;
@property (nonatomic, retain) NSColor* groupStartColor;
@property (nonatomic, retain) NSColor* groupTitleColor;
@property (nonatomic, retain) NSColor* itemSubTitleColor;
@property (nonatomic, retain) NSColor* itemTitleColor;
@property (nonatomic, retain) NSColor* itemValueTitleColor;
@property (nonatomic, retain) NSColor* listBackgroundColor;
@property (nonatomic, retain) NSColor* listStrokeColor;
@property (nonatomic, retain) NSColor* toolbarEndColor;
@property (nonatomic, retain) NSColor* toolbarShadowColor;
@property (nonatomic, retain) NSColor* toolbarStartColor;
@property (nonatomic, retain) NSColor* toolbarStrokeColor;
@property (nonatomic, retain) NSColor* toolbarTitleColor;
@property (nonatomic, retain) NSNumber * useDarkIcons;

@end
