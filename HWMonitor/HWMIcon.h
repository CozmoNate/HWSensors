//
//  HWMIcon.h
//  HWMonitor
//
//  Created by Kozlek on 16/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

#import "HWMItem.h"

@interface HWMIcon : HWMItem

@property (nonatomic, retain) NSImage * alternate;
@property (nonatomic, retain) NSImage * image;

@end
