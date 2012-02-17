/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2004 Roderick Colenbrander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <sys/stat.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

#include "nvclock.h"

void set_error(int code)
{
	nvclock.nv_errno = code;
}


void set_error_str(const char *str)
{
	nvclock.nv_errno = NV_ERR_OTHER;

	/* hacky; we need to think about memory management .. */
	nvclock.nv_err_str = /*(char*)strdup(str)*/NULL;
}


char *get_error(char *buf, int size)
{
	switch(nvclock.nv_errno)
	{
		case NV_ERR_NO_DEVICES_FOUND:
			buf=STRDUP("No nvidia cards found in your system!", sizeof("No nvidia cards found in your system!"));
			break;
		case NV_ERR_NO_DRIVERS_FOUND:
			buf=STRDUP("You don't have enough permissions to run NVClock! Retry as root or install the Nvidia drivers.", sizeof("You don't have enough permissions to run NVClock! Retry as root or install the Nvidia drivers."));
			break;
		case NV_ERR_NOT_ENOUGH_PERMISSIONS:
			buf=STRDUP("You don't have enough permissions to run NVClock! Retry as root.", sizeof("You don't have enough permissions to run NVClock! Retry as root."));
			break;
		case NV_ERR_OTHER:
			buf=STRDUP(nvclock.nv_err_str, 80);
			break;
	}

	return buf;
}
