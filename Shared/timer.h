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

static UInt64 ptimer_read()
{
    clock_sec_t secs;
    clock_nsec_t nanosecs;
    
    clock_get_calendar_nanotime(&secs, &nanosecs);
  
    return (UInt64)secs * (UInt64)NSEC_PER_SEC + (UInt64)nanosecs;
//    uint64_t uptime, nanoseconds;
//    
//    clock_get_uptime(&uptime);
//    absolutetime_to_nanoseconds(uptime, &nanoseconds);
//
//    return nanoseconds;
}

#endif
