//
//  FakeSMCNanotime.h
//  HWSensors
//
//  Created by kozlek on 09.05.12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 *	File:		clock_types.h
 *	Purpose:	Clock facility header definitions. These
 *				definitons are needed by both kernel and
 *				user-level software.
 */

/*
 *	All interfaces defined here are obsolete.
 */

#ifndef HWSensors_FakeSMCNanotime_h
#define HWSensors_FakeSMCNanotime_h

#include <kern/clock.h>

struct FakeSMCNanotime {
    clock_sec_t secs; 
    clock_nsec_t nanosecs;
};

#define NSEC_PER_USEC	1000		
#define USEC_PER_SEC	1000000		
#define NSEC_PER_SEC	1000000000	

#define SET_FAKESMC_TIMESPEC(t)                             \
clock_get_system_nanotime(&(t)->secs, &(t)->nanosecs);        

#define BAD_FAKESMC_TIMESPEC(t)                             \
((t)->nanosecs < 0 || (t)->nanosecs >= NSEC_PER_SEC)

// returned value is no more then one second (or less then minus one second) in nanoseconds!
#define CMP_FAKESMC_TIMESPEC(t1, t2)                        \
((t1)->secs > (t2)->secs ? +NSEC_PER_SEC :                  \
((t1)->secs < (t2)->secs ? -NSEC_PER_SEC :                  \
(long)((t1)->nanosecs - (t2)->nanosecs)))


#define ADD_FAKESMC_TIMESPEC(t1, t2)						\
do {														\
if (((t1)->nanosecs += (t2)->nanosecs) >= NSEC_PER_SEC) {   \
(t1)->nanosecs -= NSEC_PER_SEC;                             \
(t1)->secs  += 1;                                           \
}															\
(t1)->secs += (t2)->secs;                                   \
} while (0)


#define SUB_FAKESMC_TIMESPEC(t1, t2)						\
do {														\
if (((t1)->nanosecs -= (t2)->nanosecs) < 0) {               \
(t1)->nanosecs += NSEC_PER_SEC;                             \
(t1)->secs  -= 1;                                           \
}															\
(t1)->secs -= (t2)->secs;                                   \
} while (0)

#endif
