//
//  HWMonitorExtra.h
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.


#import <Cocoa/Cocoa.h>

#import "SystemUIPlugin.h"
#import "BundleUserDefaults.h"
#import "HWMonitorIcon.h"
#import "HWMonitorEngine.h"
#import "HWMonitorView.h"
#import "HWMonitorGroup.h"

@interface HWMonitorExtra : NSMenuExtra
{
    BundleUserDefaults* _defaults;
    NSFont *_menuFont;
    NSDictionary *_menuAttributes;
    NSDictionary *_subtitleAttributes;
    NSDictionary *_blackColorAttribute;
    NSDictionary *_orangeColorAttribute;
    NSDictionary *_redColorAttribute;
    NSImage *_prefsIcon;
    NSMutableDictionary *_icons;
    NSMutableArray *_favorites;
    
    NSMenu *_mainMenu;
    NSMutableArray *_groups;
    
    BOOL _showVolumeNames;
    BOOL _monitoringAppIsActive;
    
    float _smcSensorsUpdateInterval;
    float _smartSensorsUpdateInterval;
    
    NSDate *_smcSensorsLastUpdated;
    NSDate *_smartSensorsLastUpdated;
    
    BOOL _scheduleRebuildSensors;
}

@property (readwrite, retain) IBOutlet HWMonitorEngine* engine;
@property (readwrite, retain) IBOutlet NSArray* favorites;

@end
