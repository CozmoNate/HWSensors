//
//  HWMonitorBezierSpline.m
//  HWSensors
//
//  Created by Natan Zalkin on 22.01.13.
//
//

#import "HWMonitorBezierSpline.h"

@implementation HWMonitorBezierSpline

/// <summary>
/// Calculates scaled derivative of a point in a point collection.
/// </summary>
/// <param name="pts">Points on the curve.</param>
/// <param name="i">Point no to calculate control point for.</param>
/// <param name="a">Tension</param>
/// <returns></returns>
+ (NSPoint) getDerivativeWithPoints:(NSArray*)pts atIndex:(NSUInteger)i withTension:(double)a
{
    if ([pts count] < 2)
        [NSException raise:@"pts" format:@"MapBezier must contain at least two points."];
    
    if (i == 0)
    {
        // First point.
        return NSMakePoint(([[pts objectAtIndex:1] pointValue].x - [[pts objectAtIndex:0] pointValue].x) / a, ([[pts objectAtIndex:1] pointValue].y - [[pts objectAtIndex:0] pointValue].y) / a);
    }
    if (i == [pts count] - 1)
    {
        // Last point.
        return NSMakePoint(([[pts objectAtIndex:i] pointValue].x - [[pts objectAtIndex:i-1] pointValue].x) / a, ([[pts objectAtIndex:i] pointValue].y - [[pts objectAtIndex:i-1] pointValue].y) / a);
    }
    
    return NSMakePoint(([[pts objectAtIndex:i+1] pointValue].x - [[pts objectAtIndex:i-1] pointValue].x) / a, ([[pts objectAtIndex:i+1] pointValue].y - [[pts objectAtIndex:i-1] pointValue].y) / a);
}

/// <summary>
/// Calculates first control point of a Bézier curve.
/// </summary>
/// <param name="pts">Points on the curve.</param>
/// <param name="i">Point no to calculate control point for.</param>
/// <param name="a">Tension</param>
/// <returns></returns>
/// <remarks>Formula: B1i = Pi + Pi' / 3</remarks>
+ (NSPoint) getB1WithPoints:(NSArray*)pts atIndex:(NSUInteger)i withTension:(double)a
{
    NSPoint drv = [HWMonitorBezierSpline getDerivativeWithPoints:pts atIndex:i withTension:a];
    return NSMakePoint([[pts objectAtIndex:i] pointValue].x + drv.x / 3, [[pts objectAtIndex:i] pointValue].y + drv.y / 3);
}

/// <summary>
/// Calculates second control point of a Bézier curve.
/// </summary>
/// <param name="pts">Points on the curve.</param>
/// <param name="i">Point no to calculate control point for.</param>
/// <param name="a">Tension</param>
/// <returns></returns>
/// <remarks>Formula: B2i = P[i + 1] - P'[i + 1] / 3</remarks>
+ (NSPoint) getB2WithPoints:(NSArray*)pts atIndex:(NSUInteger)i withTension:(double)a
{
    NSPoint drv = [HWMonitorBezierSpline getDerivativeWithPoints:pts atIndex:i+1 withTension:a];
    return NSMakePoint([[pts objectAtIndex:i+1] pointValue].x - drv.x / 3, [[pts objectAtIndex:i+1] pointValue].y - drv.y / 3);
}

/// <summary>
/// Calculates Bézier control points for a curve that contains given set of points.
/// </summary>
/// <param name="pts">Points on the curve.</param>
/// <param name="tension">Tension.</param>
/// <returns></returns>
+ (NSArray*) getBezierPointsForGraph:(NSArray*)pts withTension:(double)tension
{
    NSMutableArray* ret = [[NSMutableArray alloc] init];
    
    for (int i = 0; i < [pts count]; i++)
    {
        // for first point append as is.
        if (i == 0)
        {
            [ret addObject:[NSValue valueWithPoint:NSMakePoint([[pts objectAtIndex:0] pointValue].x, [[pts objectAtIndex:0] pointValue].y)]];
            continue;
        }
        
        // for each point except first and last get B1, B2. next point.
        // Last point do not have a next point.
        [ret addObject:[NSValue valueWithPoint:[HWMonitorBezierSpline getB1WithPoints:pts atIndex:i - 1 withTension:tension]]];
        [ret addObject:[NSValue valueWithPoint:[HWMonitorBezierSpline getB2WithPoints:pts atIndex:i - 1 withTension:tension]]];
        [ret addObject:[NSValue valueWithPoint:[[pts objectAtIndex:i] pointValue]]];
    }
    
    return [NSArray arrayWithArray:ret];
}

@end
