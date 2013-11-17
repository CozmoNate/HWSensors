//
//  HWMEngine.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface HWMEngine : NSObject
{
    NSArray *_platformProfile;
    io_connect_t _smcConnection;
    io_connect_t _fakeSmcConnection;
}

@property (nonatomic, strong) NSBundle * bundle;

@property (nonatomic, strong) NSManagedObjectModel * managedObjectModel;
@property (nonatomic, strong) NSPersistentStoreCoordinator * persistentStoreCoordinator;
@property (nonatomic, strong) NSManagedObjectContext * managedObjectContext;

+(HWMEngine*)engineWithBundle:(NSBundle*)bundle;

-(void)rebuildSensorsList;

@end
