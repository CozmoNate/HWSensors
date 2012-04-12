/*
 *  TigerAdditionals.h
 *  VoodooHDA
 *
 *  Created by Andy Vandijck on 12/03/10.
 *  Copyright 2010 AnV Software. All rights reserved.
 *
 */

#ifdef TIGER // needed for compile fix...
int vprintf_vhda(const char *fmt, va_list ap);
int snprintf_vhda(char *text, size_t maxlen, const char *fmt, ...);

#define vprintf vprintf_vhda
#define snprintf snprintf_vhda
#define strlcpy strncpy
#define strlcat strncat

enum {
    kIOUCVariableStructureSize = 0xffffffff
};
#endif

