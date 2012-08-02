//
//  HWMonitorView.h
//  HWSensors
//
// Based on code by:
//
//  DeVercruesseASMView.m
//
//  Created by Frank Vercruesse on Sat Aug 25 2001.
//  Copyright (c) 2001 Frank Vercruesse.
//
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


#import "SystemUIPlugin.h"
#import "HWMonitorEngine.h"

@interface HWMonitorView : NSMenuExtraView
{
    NSFont *_smallFont;
    NSFont *_bigFont;
    NSShadow *_shadow;
}

@property (readwrite, retain) HWMonitorEngine *engine;
@property (readwrite, retain) NSArray *favorites;

@property (readwrite, assign) BOOL useBigFont;
@property (readwrite, assign) BOOL useShadowEffect;

@end
