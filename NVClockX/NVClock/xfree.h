/* NVTV xfree -- Dirk Thierbach <dthierbach@gmx.de>
 *
 * Header: All definitions from xfree that are needed.
 *
 */

#ifndef _XFREE_H
#define _XFREE_H 1

//#include <stdio.h>
//#include <stdlib.h>

#define xf86Msg(type,format,args...) /* */
#define xf86DrvMsg(scrnIndex,type,format, args...) /* */

#ifndef Bool
#  ifndef _XTYPEDEF_BOOL
#   define _XTYPEDEF_BOOL
typedef int Bool;
#  endif
#endif

#ifndef _XTYPEDEF_POINTER
#  define _XTYPEDEF_POINTER
typedef void *pointer;
#endif


/* Flags for driver messages */
typedef enum {
    X_PROBED,			/* Value was probed */
    X_CONFIG,			/* Value was given in the config file */
    X_DEFAULT,			/* Value is a default */
    X_CMDLINE,			/* Value was given on the command line */
    X_NOTICE,			/* Notice */
    X_ERROR,			/* Error message */
    X_WARNING,			/* Warning message */
    X_INFO,			/* Informational message */
    X_NONE,			/* No prefix */
    X_NOT_IMPLEMENTED		/* Not implemented */
} MessageType;

typedef union _DevUnion {
    pointer             ptr;
    long                val;
    unsigned long       uval;
    pointer             (*fptr)(void);
} DevUnion;


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

void xf86usleep(unsigned long usec);
void xf86getsecs(long * secs, long * usecs);

#define xcalloc(_num, _size) calloc(_num, _size)
#define xfree(_ptr) free(_ptr)


/* ---------------- nv driver files ---------------- */

/**** nv_dac.c */

#define DDC_SDA_READ_MASK  (1 << 3)
#define DDC_SCL_READ_MASK  (1 << 2)
#define DDC_SDA_WRITE_MASK (1 << 4)
#define DDC_SCL_WRITE_MASK (1 << 5)


#endif
