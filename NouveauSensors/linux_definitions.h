//
//  linux_definitions.h
//  HWSensors
//
//  Created by Kozlek on 13.08.12.
//
//

#ifndef HWSensors_linux_definitions_h
#define HWSensors_linux_definitions_h

#include <IOKit/IOLib.h>

#define EPERM           1                // Operation not permitted
#define ENOENT          2                // No such file or directory
#define ESRCH           3                // No such process
#define EINTR           4                // Interrupted system call
#define EIO             5                // Input/output error
#define ENXIO           6                // Device not configured
#define E2BIG           7                // Argument list too long
#define ENOEXEC         8                // Exec format error
#define EBADF           9                // Bad file number
#define ECHILD          10               // No spawned processes
#define EAGAIN          11               // Resource temporarily unavailable
#define ENOMEM          12               // Cannot allocate memory
#define EACCES          13               // Access denied
#define EFAULT          14               // Bad address
#define ENOTBLK         15               // Not block device
#define EBUSY           16               // Device busy
#define EEXIST          17               // File exist
#define EXDEV           18               // Cross-device link
#define ENODEV          19               // Operation not supported by device
#define ENOTDIR         20               // Not a directory
#define EISDIR          21               // Is a directory
#define EINVAL          22               // Invalid argument
#define ENFILE          23               // Too many open files in system
#define EMFILE          24               // Too many files open
#define ENOTTY          25               // Inappropriate ioctl for device
#define ETXTBSY         26               // Unknown error
#define EFBIG           27               // File too large
#define ENOSPC          28               // No space left on device
#define ESPIPE          29               // Illegal seek
#define EROFS           30               // Read-only file system
#define EMLINK          31               // Too many links
#define EPIPE           32               // Broken pipe
#define EDOM            33               // Numerical arg out of domain
#define ERANGE          34               // Result too large
#define EUCLEAN         35               // Structure needs cleaning
#define EDEADLK         36               // Resource deadlock avoided
#define EUNKNOWN        37               // Unknown error
#define ENAMETOOLONG    38               // File name too long
#define ENOLCK          39               // No locks available
#define ENOSYS          40               // Function not implemented
#define ENOTEMPTY       41               // Directory not empty
#define EILSEQ          42               // Invalid multibyte sequence

//
// Sockets errors
//

#define EWOULDBLOCK     45               // Operation would block
#define EINPROGRESS     46               // Operation now in progress
#define EALREADY        47               // Operation already in progress
#define ENOTSOCK        48               // Socket operation on nonsocket
#define EDESTADDRREQ    49               // Destination address required
#define EMSGSIZE        50               // Message too long
#define EPROTOTYPE      51               // Protocol wrong type for socket
#define ENOPROTOOPT     52               // Bad protocol option
#define EPROTONOSUPPORT 53               // Protocol not supported
#define ESOCKTNOSUPPORT 54               // Socket type not supported
#define EOPNOTSUPP      55               // Operation not supported
#define EPFNOSUPPORT    56               // Protocol family not supported
#define EAFNOSUPPORT    57               // Address family not supported
#define EADDRINUSE      58               // Address already in use
#define EADDRNOTAVAIL   59               // Cannot assign requested address
#define ENETDOWN        60               // Network is down
#define ENETUNREACH     61               // Network is unreachable
#define ENETRESET       62               // Network dropped connection on reset
#define ECONNABORTED    63               // Connection aborted
#define ECONNRESET      64               // Connection reset by peer
#define ENOBUFS         65               // No buffer space available
#define EISCONN         66               // Socket is already connected
#define ENOTCONN        67               // Socket is not connected
#define ESHUTDOWN       68               // Cannot send after socket shutdown
#define ETOOMANYREFS    69               // Too many references
#define ETIMEDOUT       70               // Operation timed out
#define ECONNREFUSED    71               // Connection refused
#define ELOOP           72               // Cannot translate name
#define EWSNAMETOOLONG  73               // Name component or name was too long
#define EHOSTDOWN       74               // Host is down
#define EHOSTUNREACH    75               // No route to host
#define EWSNOTEMPTY     76               // Cannot remove a directory that is not empty
#define EPROCLIM        77               // Too many processes
#define EUSERS          78               // Ran out of quota
#define EDQUOT          79               // Ran out of disk quota
#define ESTALE          80               // File handle reference is no longer available
#define EREMOTE         81               // Item is not available locally

//
// Resolver errors
//

#define EHOSTNOTFOUND   82               // Host not found
#define ETRYAGAIN       83               // Nonauthoritative host not found
#define ENORECOVERY     84               // A nonrecoverable error occured
#define ENODATA         85               // Valid name, no data record of requested type

//
// Misc. error codes
//

#define EPROTO          86               // Protocol error
#define ECHKSUM         87               // Checksum error
#define EBADSLT         88               // Invalid slot
#define EREMOTEIO       89               // Remote I/O error

//
// Error code aliases
//

#define ETIMEOUT        ETIMEDOUT
#define EBUF            ENOBUFS
#define EROUTE          ENETUNREACH
#define ECONN           ENOTCONN
#define ERST            ECONNRESET
#define EABORT          ECONNABORTED

typedef UInt8 u8;
typedef UInt16 u16;
typedef UInt32 u32;
typedef UInt64 u64;
typedef SInt8 s8;
typedef SInt16 s16;
typedef SInt32 s32;
typedef SInt64 s64;

#define pr_err(format, args...) do {IOLog("LinuxI2C(pr_err): " format, ##args);} while (0) //fprintf (stderr, format, ## __VA_ARGS__)
#define pr_debug(format, args...) do {IOLog("LinuxI2C(pr_debug): " format, ##args);} while (0)
#define dev_err(dev, format, args...) do {IOLog("LinuxI2C(dev_err): " format, ##args);} while (0) //fprintf (stderr, format, ## __VA_ARGS__)
#define dev_warn(dev, format, args...) do {IOLog("LinuxI2C(dev_warn): " format, ##args);} while (0) //fprintf (stderr, format, ## __VA_ARGS__)

#define printk(format, args...) do {IOLog("LinuxI2C(printk): " format, ##args);} while (0)

#define pr_info(format, args...) do {IOLog("LinuxI2C(pr_info): " format, ##args);} while (0)

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
static inline void rep_nop(void)
{
    __asm__ __volatile__("rep;nop": : :"memory");
}

#define cpu_relax()   rep_nop()

#define I2C_NAME_SIZE	20

#endif
