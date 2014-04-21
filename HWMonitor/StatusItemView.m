//
//  StatusItemView.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
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

#import "StatusItemView.h"

#import "HWMonitorDefinitions.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMIcon.h"
#import "HWMSensor.h"
#import "HWMFavorite.h"

#import <ReactiveCocoa/ReactiveCocoa.h>

@interface StatusItemView ()

@property (readonly) NSAttributedString *spacer;
@property (readonly) NSArray *favoritesSnapshot;

@end

@implementation StatusItemView

@synthesize spacer = _spacer;
@synthesize favoritesSnapshot = _favoritesSnapshot;

#pragma mark -
#pragma mark Properties

-(NSAttributedString *)spacer
{
    if (!_spacer) {
        _spacer = [[NSAttributedString alloc] initWithString:self.monitorEngine.configuration.useBigFontInMenubar.boolValue ? @" " : @"  " attributes:[NSDictionary dictionaryWithObjectsAndKeys:self.monitorEngine.configuration.useBigFontInMenubar.boolValue ? _bigFont : _smallFont, NSFontAttributeName, nil]];
    }

    return _spacer;
}

-(NSArray *)favoritesSnapshot
{
    if (!_favoritesSnapshot) {
        _favoritesSnapshot = [self.monitorEngine.favorites copy];
    }

    return _favoritesSnapshot;
}

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
}

- (void)setHighlighted:(BOOL)isHighlighted
{
    if (_isHighlighted != isHighlighted) {
        _isHighlighted = isHighlighted;
    }
}

#pragma mark -
#pragma mark Override

-(id)initWithFrame:(NSRect)rect statusItem:(NSStatusItem *)statusItem;
{
    self = [super initWithFrame:rect];

    if (self && statusItem) {
        _statusItem = statusItem;

        _smallFont = [NSFont fontWithName:@"Lucida Grande Bold" size:9.0];
        _bigFont = [NSFont fontWithName:@"Lucida Grande" size:13.9];

        _shadow = [[NSShadow alloc] init];

        [_shadow setShadowColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.50]];
        [_shadow setShadowOffset:CGSizeMake(0, -1.0)];
        [_shadow setShadowBlurRadius:1.0];

        [_statusItem setView:self];

        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [[RACObserve(self, monitorEngine) filter:^BOOL(id value) {
                return value != nil;
            }] subscribeNext:^(HWMEngine *engine) {

                RACSignal *favoritesChangedSignal = [RACObserve(engine, favorites) distinctUntilChanged];

                [favoritesChangedSignal subscribeNext:^(id x) {
                    _favoritesSnapshot = nil;
                }];

                RACSignal *bigFontChangedSignal = RACObserve(engine.configuration, useBigFontInMenubar);

                [bigFontChangedSignal subscribeNext:^(id x) {
                    _spacer = nil;
                }];

                [[RACSignal combineLatest:@[favoritesChangedSignal,
                                            bigFontChangedSignal,
                                            RACObserve(engine.configuration, useShadowEffectsInMenubar),
                                            RACObserve(engine.configuration, useFahrenheit)]]
                 subscribeNext:^(id x) {
                     [[NSOperationQueue mainQueue] addOperationWithBlock:^{
                         [self setNeedsDisplay:YES];
                     }];
                 }];
            }];

            [[[NSNotificationCenter defaultCenter] rac_addObserverForName:HWMEngineSensorValuesHasBeenUpdatedNotification object:nil] subscribeNext:^(id x) {
                [[NSOperationQueue mainQueue] addOperationWithBlock:^{
                    [self setNeedsDisplay:YES];
                }];
            }];
        }];
    }

    return self;
}

- (void)drawRect:(NSRect)rect
{
    //[_statusItem drawStatusBarBackgroundInRect:rect withHighlight:_isHighlighted];

    CGContextRef cgContext = [[NSGraphicsContext currentContext] graphicsPort];

    CGContextSetShouldSmoothFonts(cgContext, !self.monitorEngine.configuration.useBigFontInMenubar.boolValue);

    __block int offset = 3;

    if (self.monitorEngine) {

        if (!self.favoritesSnapshot.count) {

            [[NSGraphicsContext currentContext] saveGraphicsState];

            if (/*!_isHighlighted &&*/ self.monitorEngine.configuration.useShadowEffectsInMenubar.boolValue)
                [_shadow set];

            NSImage *image = /*_isHighlighted ? _alternateImage :*/ _image;

            if (image) {
                NSUInteger width = image.size.width + 12;

                [image drawAtPoint:NSMakePoint(lround((width - image.size.width) / 2), lround((self.frame.size.height - image.size.height) / 2)) fromRect:NSMakeRect(0, 0, width, image.size.height) operation:NSCompositeSourceOver fraction:1.0];

                [self setFrameSize:NSMakeSize(width, self.frame.size.height)];
            }

            [[NSGraphicsContext currentContext] restoreGraphicsState];

            //snow leopard icon & text problem
            [_statusItem setLength:(self.frame.size.width)];

            return;
        }

        __block int lastWidth = 0;
        __block int index = 0;

        [self.favoritesSnapshot enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {

            HWMFavorite *favorite = (HWMFavorite *)obj;

            HWMFavorite *next = idx + 1 < self.favoritesSnapshot.count ? [self.favoritesSnapshot objectAtIndex:idx + 1] : nil;

            HWMItem *item = favorite.item;

            if ([item isKindOfClass:[HWMIcon class]]) {

                HWMIcon *icon = (HWMIcon*)item;

                if (self.favoritesSnapshot.count == 1) {
                    offset += 3;
                }

                [[NSGraphicsContext currentContext] saveGraphicsState];

                NSImage *image = /*_isHighlighted ? [icon alternateImage] :*/ icon.regular;

                if (image) {

                    if (image.isTemplate && self.monitorEngine.configuration.useShadowEffectsInMenubar.boolValue)
                        [_shadow set];

                    [image drawAtPoint:NSMakePoint(offset, lround((self.frame.size.height - image.size.height) / 2)) fromRect:NSMakeRect(0, 0, image.size.width, image.size.height) operation:NSCompositeSourceOver fraction:1.0];

                    offset = offset + image.size.width + (next && [next.item isKindOfClass:[HWMSensor class]] ? 2 : idx + 1 == self.favoritesSnapshot.count ? 0 : self.spacer.size.width);

                    index = 0;
                }

                [[NSGraphicsContext currentContext] restoreGraphicsState];

                if (self.favoritesSnapshot.count == 1) {
                    offset += 3;
                }
            }
            else if ([item isKindOfClass:[HWMSensor class]]) {

                HWMSensor *sensor = (HWMSensor*)item;

                NSMutableAttributedString * title = [[NSMutableAttributedString alloc] initWithString:sensor.strippedValue];

                NSColor *valueColor;

                switch (sensor.alarmLevel) {
                    case kHWMSensorLevelModerate:
                        valueColor = [NSColor colorWithCalibratedRed:0.7f green:0.3f blue:0.03f alpha:1.0f];
                        break;

                    case kHWMSensorLevelHigh:
                    case kHWMSensorLevelExceeded:
                        valueColor = [NSColor redColor];
                        break;

                    default:
                        valueColor = [NSColor blackColor];
                        break;
                }

                [title addAttribute:NSForegroundColorAttributeName value:(/*_isHighlighted ? [NSColor whiteColor] :*/ valueColor) range:NSMakeRange(0,title.length)];

                [[NSGraphicsContext currentContext] saveGraphicsState];

                if (/*!_isHighlighted &&*/ self.monitorEngine.configuration.useShadowEffectsInMenubar.boolValue)
                    [_shadow set];

                if (favorite.large.boolValue || self.monitorEngine.configuration.useBigFontInMenubar.boolValue) {

                    [title addAttribute:NSFontAttributeName value:_bigFont range:NSMakeRange(0, title.length)];

                    [title drawAtPoint:NSMakePoint(offset, lround((self.frame.size.height - title.size.height) / 2))];

                    offset += title.size.width  + (idx + 1 < self.favoritesSnapshot.count ? self.spacer.size.width : 0);

                    index = 0;
                }
                else {
                    int row = index % 2;

                    [title addAttribute:NSFontAttributeName value:_smallFont range:NSMakeRange(0, title.length)];

                    [title drawAtPoint:NSMakePoint(offset, self.favoritesSnapshot.count == 1 ? lround((self.frame.size.height - title.size.height) / 2) + 1 : row == 0 ? lround(self.frame.size.height / 2) - 1 : lround(self.frame.size.height / 2) - title.size.height + 2)];

                    int width = title.size.width;

                    if (row == 0) {
                        if (idx + 1 == self.favoritesSnapshot.count) {
                            offset += width;
                        }
                        else if (next && !([next.item isKindOfClass:[HWMSensor class]] && !next.large.boolValue)) {
                            offset += width + self.spacer.size.width;
                        }
                    }
                    else if (row == 1) {
                        width = width > lastWidth ? width : lastWidth;
                        offset += width + (idx + 1 < self.favoritesSnapshot.count ? self.spacer.size.width : 0);
                    }
                    
                    lastWidth = width;
                    
                    index++;
                }
                
                [[NSGraphicsContext currentContext] restoreGraphicsState];
            }

        }];
    }


    offset += 3;

    [self setFrameSize:NSMakeSize(offset, self.frame.size.height)];

    //snow leopard icon & text problem
    [_statusItem setLength:(self.frame.size.width)];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [NSApp sendAction:self.action to:self.target from:self];
}

#pragma mark -
#pragma mark Methods

-(NSRect)screenRect
{
    return [self.window convertRectToScreen:self.frame];
}

@end
