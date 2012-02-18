/*
 *  TigerAdditionals.cpp
 *  VoodooHDA
 *
 *  Created by Andy Vandijck on 12/03/10.
 *  Copyright 2010 AnV Software. All rights reserved.
 *
 *  Slice, 05.10.2010 some additions
 */

#ifdef TIGER // needed for compile fix
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/msgbuf.h>
#include <sys/proc_internal.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/file_internal.h>
#include <sys/syslog.h>
#include <stdarg.h>
#include <sys/malloc.h>
#include <sys/lock.h>

#include <machine/spl.h>
#include <libkern/libkern.h>

int
vprintf_vhda(const char *fmt, va_list ap)
{
	char tmp[65536]; /* Should be enough, right? */

	vsnprintf(tmp, sizeof(tmp), fmt, ap);
	printf("%s", tmp);

	return 0;
}

int snprintf_vhda(char *text, size_t maxlen, const char *fmt, ...)
{
    va_list ap;
    int retval;
	
    va_start(ap, fmt);
    retval = vsnprintf(text, maxlen, fmt, ap);
    va_end(ap);
	
    return retval;
}

#endif
