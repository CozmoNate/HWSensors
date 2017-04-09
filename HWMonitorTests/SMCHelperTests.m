//
//  HWMonitorTests.m
//  HWMonitorTests
//
//  Created by Natan Zalkin on 08/04/2017.
//  Copyright © 2017 kozlek. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "SmcHelper.h"
#import "FakeSMCDefinitions.h"

@interface SMCHelperTests : XCTestCase

@end

@implementation SMCHelperTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testGetIndexFromChar {
    
    char c = '3';
    
    XCTAssertEqual([SmcHelper getIndexFromHexChar:c], 3);
}

- (void)testIntTypeCheckValidity
{
    XCTAssertTrue([SmcHelper isValidIntegerSmcType:@SMC_TYPE_UI8]);
    XCTAssertTrue([SmcHelper isValidIntegerSmcType:@SMC_TYPE_UI16]);
    XCTAssertTrue([SmcHelper isValidIntegerSmcType:@SMC_TYPE_UI32]);
    XCTAssertTrue([SmcHelper isValidIntegerSmcType:@SMC_TYPE_SI8]);
    XCTAssertTrue([SmcHelper isValidIntegerSmcType:@SMC_TYPE_SI16]);
    XCTAssertTrue([SmcHelper isValidIntegerSmcType:@SMC_TYPE_SI32]);
}

- (void)testIntTypeCheckInvalidity
{
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_FP2E]);
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_FP4C]);
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_FP5B]);
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_FP88]);
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_SP4B]);
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_SP78]);
    
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_FDS]);
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_CH8]);
    XCTAssertFalse([SmcHelper isValidIntegerSmcType:@SMC_TYPE_FLAG]);
}

- (void)testFixedPointTypeCheckValidity
{
    XCTAssertTrue([SmcHelper isValidFloatingSmcType:@SMC_TYPE_FP2E]);
    XCTAssertTrue([SmcHelper isValidFloatingSmcType:@SMC_TYPE_FP4C]);
    XCTAssertTrue([SmcHelper isValidFloatingSmcType:@SMC_TYPE_FP5B]);
    XCTAssertTrue([SmcHelper isValidFloatingSmcType:@SMC_TYPE_FP88]);
    XCTAssertTrue([SmcHelper isValidFloatingSmcType:@SMC_TYPE_SP4B]);
    XCTAssertTrue([SmcHelper isValidFloatingSmcType:@SMC_TYPE_SP78]);
}

- (void)testFixedPointTypeCheckInvalidity
{
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_UI8]);
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_UI16]);
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_UI32]);
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_SI8]);
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_SI16]);
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_SI32]);
    
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_FDS]);
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_CH8]);
    XCTAssertFalse([SmcHelper isValidFloatingSmcType:@SMC_TYPE_FLAG]);
}

- (void)testUnsignedFloatValueEncodeDecodeAndSpeed {
    
    const char * type = SMC_TYPE_FP4C;
    const int size = SMC_TYPE_FPXX_SIZE;
    const int shift = BIT(0xc);
    
    [self measureBlock:^{
        
        void * buffer[size];
        
        for (int i = 0; i < 1e7; i++) {
            
            float testValue = arc4random_uniform(10) / (arc4random_uniform(9) + 1);
            float resultValue = (float)((int)(testValue * shift) / (float)shift);
    
            XCTAssertTrue([SmcHelper encodeNumericValue:@(testValue) length:size type:type outBuffer:&buffer]);
            XCTAssertEqual([SmcHelper decodeNumericValueFromBuffer:buffer length:size type:type].floatValue, resultValue);
        }
    }];
}

- (void)testUnsignedIntValueEncodeDecodeAndSpeed {
    
    const char * type = SMC_TYPE_UI32;
    const int size = SMC_TYPE_UI32_SIZE;
    
    [self measureBlock:^{
        
        void * buffer[size];
        
        for (int i = 0; i < 1e7; i++) {
            
            int testValue = arc4random_uniform(BIT(31));
            
            XCTAssertTrue([SmcHelper encodeNumericValue:@(testValue) length:size type:type outBuffer:&buffer]);
            XCTAssertEqual([SmcHelper decodeNumericValueFromBuffer:buffer length:size type:type].intValue, testValue);
        }
    }];
}

// TODO: Add test for signed int and float

@end
