//
//  HWMIcon.m
//  HWMonitor
//
//  Created by Kozlek on 16/11/13.
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

#import "HWMIcon.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMEngine.h"

#import <ReactiveCocoa/ReactiveCocoa.h>

@implementation HWMIcon

@dynamic alternate;
@dynamic regular;

@synthesize themedImage;

-(NSImage *)themedImage
{
    return self.engine.configuration.colorTheme.useBrightIcons.boolValue ? self.alternate : self.regular;
}

-(void)initialize
{
    [super initialize];

    [[RACObserve(self, engine)
       filter:^BOOL(HWMEngine *engine) {
           return engine != nil;
       }]
     subscribeNext:^(HWMEngine *engine) {
         [[RACObserve(engine.configuration, colorTheme) filter:^BOOL(id value) {
             return value != nil;
         }] subscribeNext:^(HWMColorTheme *theme) {
             [self willChangeValueForKey:@keypath(self, themedImage)];
             [self didChangeValueForKey:@keypath(self, themedImage)];
         }];
     }];
}

@end
