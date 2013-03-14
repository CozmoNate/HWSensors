//
//  AppDelegate.h
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

#import <Cocoa/Cocoa.h>

#import "PopupController.h"
#import "GraphsController.h"

#import "BundleUserDefaults.h"
#import "HWMonitorIcon.h"
#import "HWMonitorEngine.h"

@interface AppController : NSWindowController <NSApplicationDelegate, NSTableViewDataSource, NSTableViewDelegate, PopupControllerDelegate>
{
    BundleUserDefaults* _defaults;
    HWMonitorEngine *_engine;
    NSMutableDictionary *_icons;
    NSMutableArray* _favorites;
    NSMutableArray *_groups;
    NSMutableDictionary *_items;
    NSMutableDictionary *_index;
    NSMutableArray *_ordering;
    
    float _smcSensorsUpdateInterval;
    float _smartSensorsUpdateInterval;
    NSDate *_favoritesSensorsLastUpdated;
    NSDate *_smcSensorsLastUpdated;
    NSDate *_smartSensorsLastUpdated;
    BOOL _scheduleRebuildSensors;
    
    id _favoriteGroupItem;
    id _availableGroupItem;
    
    NSArray *_colorThemes;
}

@property (nonatomic, unsafe_unretained) IBOutlet PopupController *popupController;
@property (nonatomic, unsafe_unretained) IBOutlet GraphsController *graphsController;

@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *smcUpdateRateTextField;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *smartUpdateRateTextField;

@property (nonatomic, unsafe_unretained) IBOutlet NSTableView *sensorsTableView;

- (IBAction)favoritesChanged:(id)sender;
- (IBAction)useFahrenheitChanged:(id)sender;
- (IBAction)colorThemeChanged:(id)sender;
- (IBAction)updateRateChanged:(id)sender;
- (IBAction)toggleSensorVisibility:(id)sender;
- (IBAction)useBigFontChanged:(id)sender;
- (IBAction)useShadowEffectChanged:(id)sender;
- (IBAction)useBSDNamesChanged:(id)sender;
- (IBAction)showVolumeNamesChanged:(id)sender;
- (IBAction)toggleGraphSmoothing:(id)sender;

@end
