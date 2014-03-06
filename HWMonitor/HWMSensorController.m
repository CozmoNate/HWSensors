//
//  HWMSensorController.m
//  HWMonitor
//
//  Created by Kozlek on 01/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "HWMSensorController.h"
#import "HWMSensor.h"


@implementation HWMSensorController

@dynamic enabled;
@dynamic output;
@dynamic input;

-(void)setInput:(HWMSensor *)input
{
    [self willChangeValueForKey:@"input"];

    if (self.input) {
        [self removeObserver:self forKeyPath:@"input.value"];
    }

    [self setPrimitiveValue:input forKey:@"input"];

    if (input) {
        [self addObserver:self forKeyPath:@"input.value" options:NSKeyValueObservingOptionNew context:nil];
    }

    [self didChangeValueForKey:@"input"];
}

-(void)inputValueChanged
{

}

-(void)awakeFromFetch
{
    [super awakeFromFetch];

    if (self.input) {
        [self addObserver:self forKeyPath:@"input.value" options:NSKeyValueObservingOptionNew context:nil];
    }
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];

    if (self.input) {
        [self addObserver:self forKeyPath:@"input.value" options:NSKeyValueObservingOptionNew context:nil];
    }
}

-(void)prepareForDeletion
{
    [super prepareForDeletion];

    if (self.input) {
        [self removeObserver:self forKeyPath:@"input.value"];
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"input.value"]) {
        if (self.enabled.boolValue) {
            [self inputValueChanged];
        }
    }
}

@end
