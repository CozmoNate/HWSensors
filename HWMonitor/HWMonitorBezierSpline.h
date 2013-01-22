//
//  HWMonitorBezierSpline.h
//  HWSensors
//
//  Created by Natan Zalkin on 22.01.13.
//
//  Ported from code by Colin Eberhardt, November 22nd, 2010
//  http://www.scottlogic.co.uk/blog/colin/2010/11/adding-a-smoothed-line-series-bezier-curve-to-a-visiblox-chart/
//

#import <Foundation/Foundation.h>

@interface HWMonitorBezierSpline : NSObject

+ (NSPoint) getDerivativeWithPoints:(NSArray*)pts atIndex:(NSUInteger)i withTension:(double)a;
+ (NSPoint) getB1WithPoints:(NSArray*)pts atIndex:(NSUInteger)i withTension:(double)a;
+ (NSPoint) getB2WithPoints:(NSArray*)pts atIndex:(NSUInteger)i withTension:(double)a;
+ (NSArray*) getBezierPointsForGraph:(NSArray*)pts withTension:(double)tension;

@end
