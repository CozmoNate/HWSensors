//
//  timer.h
//  HWSensors
//
//  Created by Kozlek on 28.09.12.
//
//

#ifndef HWSensors_timer_h
#define HWSensors_timer_h

#include <kern/clock.h>

inline UInt64 ptimer_read()
{
    clock_sec_t secs;
    clock_nsec_t nanosecs;
    
    clock_get_calendar_nanotime(&secs, &nanosecs);
  
    return (UInt64)secs * (UInt64)NSEC_PER_SEC + (UInt64)nanosecs;
}

inline double ptimer_read_seconds()
{
    clock_sec_t secs;
    clock_usec_t microsecs;
    
    clock_get_calendar_microtime(&secs, &microsecs);
    
    return (double)secs + (double)microsecs / (double)USEC_PER_SEC;
}

#endif
