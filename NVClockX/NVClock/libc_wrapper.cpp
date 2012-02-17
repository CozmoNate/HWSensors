/* Excerpt from: */

/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/shared/libc_wrapper.c,v 1.70 2000/09/26 15:57:20 tsi Exp $ */
/*
 * Copyright 1997 by The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Orest Zborowski and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Orest Zborowski
 * and David Wexelblat make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THE XFREE86 PROJECT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL OREST ZBOROWSKI OR DAVID WEXELBLAT BE LIABLE 
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

//#include <stdio.h>
#include "nvclock.h"
#include "xfree.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/time.h>

/* Misc functions. Some are ANSI C, some are not. */

/* FIXME might use nanosleep (POSIX) here. */
/* resolution of 1/HZ s (i.e, 10 ms on Linux/i386 and 1 ms on Linux/Alpha). */
/* If  the process is scheduled under a real-time policy like
       SCHED_FIFO or SCHED_RR, then pauses of up to 2 ms will  be
       performed as busy waits with microsecond precision.
*/

void
xf86usleep(unsigned long usec)
    //unsigned long usec;
{
	IOSleep(usec);
}


void xf86getsecs(long * secs, long * usecs)
{
	/*struct timeval tv;

	microtime(&tv);
	*secs = tv.tv_sec;
	*usecs= tv.tv_usec;
*/
	return;
}

