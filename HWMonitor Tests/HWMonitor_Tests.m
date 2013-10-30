//
//  HWMonitor_Tests.m
//  HWMonitor Tests
//
//  Created by Kozlek on 30/10/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <XCTest/XCTest.h>

//#import "HWMonitorProfiles.h"

@interface HWMonitor_Tests : XCTestCase

@end

@implementation HWMonitor_Tests

- (void)setUp
{
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample
{
//    [[HWMonitorProfiles profiles] enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
//        NSString *name = (NSString*)key;
//        NSArray *profile = (NSArray*)obj;
//
//        NSMutableArray *adopted = [[NSMutableArray alloc] init];
//
//        [profile enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
//            NSArray *entry = (NSArray*)obj;
//            //[adopted setObject:[entry objectAtIndex:1] forKey:[entry objectAtIndex:0]];
//            [adopted addObject:[NSString stringWithFormat:@"%@|%@", (NSString*)[entry objectAtIndex:0], (NSString*)[entry objectAtIndex:1]]];
//        }];
//
//        [adopted writeToFile:[NSString stringWithFormat:@"HWMonitor/Profiles/%@", [name stringByAppendingPathExtension:@"plist"]] atomically:YES];
//    }];
    XCTFail(@"No implementation for \"%s\"", __PRETTY_FUNCTION__);
}

@end
