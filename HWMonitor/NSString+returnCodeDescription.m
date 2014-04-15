//
//  NSString+returnCodeDescription.m
//  HWMonitor
//
//  Created by Kozlek on 15.04.14.
//  Copyright (c) 2014 emeraldo.cs, kozlek. All rights reserved.
//

// Based on code from: http://stackoverflow.com/questions/3887309/mapping-iokit-ioreturn-error-code-to-string

#import "NSString+returnCodeDescription.h"

@implementation NSString (returnCodeDescription)

+(NSString *)stringFromReturn:(IOReturn)code
{
    static dispatch_once_t onceToken;
    static NSDictionary *descriptionMap;

    dispatch_once(&onceToken, ^{
        descriptionMap =  @{@kIOReturnSuccess:          @"success",
                            @kIOReturnError:            @"general error",
                            @kIOReturnNoMemory:         @"memory allocation error",
                            @kIOReturnNoResources:      @"resource shortage",
                            @kIOReturnIPCError:         @"Mach IPC failure",
                            @kIOReturnNoDevice:         @"no such device",
                            @kIOReturnNotPrivileged:    @"privilege violation",
                            @kIOReturnBadArgument:      @"invalid argument",
                            @kIOReturnLockedRead:       @"device is read locked",
                            @kIOReturnLockedWrite:      @"device is write locked",
                            @kIOReturnExclusiveAccess:  @"device is exclusive access",
                            @kIOReturnBadMessageID:     @"bad IPC message ID",
                            @kIOReturnUnsupported:      @"unsupported function",
                            @kIOReturnVMError:          @"virtual memory error",
                            @kIOReturnInternalError:    @"internal driver error",
                            @kIOReturnIOError:          @"I/O error",
                            @kIOReturnCannotLock:       @"cannot acquire lock",
                            @kIOReturnNotOpen:          @"device is not open",
                            @kIOReturnNotReadable:      @"device is not readable",
                            @kIOReturnNotWritable:      @"device is not writeable",
                            @kIOReturnNotAligned:       @"alignment error",
                            @kIOReturnBadMedia:         @"media error",
                            @kIOReturnStillOpen:        @"device is still open",
                            @kIOReturnRLDError:         @"rld failure",
                            @kIOReturnDMAError:         @"DMA failure",
                            @kIOReturnBusy:             @"device is busy",
                            @kIOReturnTimeout:          @"I/O timeout",
                            @kIOReturnOffline:          @"device is offline",
                            @kIOReturnNotReady:         @"device is not ready",
                            @kIOReturnNotAttached:      @"device/channel is not attached",
                            @kIOReturnNoChannels:       @"no DMA channels available",
                            @kIOReturnNoSpace:          @"no space for data",
                            @kIOReturnPortExists:       @"device port already exists",
                            @kIOReturnCannotWire:       @"cannot wire physical memory",
                            @kIOReturnNoInterrupt:      @"no interrupt attached",
                            @kIOReturnNoFrames:         @"no DMA frames enqueued",
                            @kIOReturnMessageTooLarge:  @"message is too large",
                            @kIOReturnNotPermitted:     @"operation is not permitted",
                            @kIOReturnNoPower:          @"device is without power",
                            @kIOReturnNoMedia:          @"media is not present",
                            @kIOReturnUnformattedMedia: @"media is not formatted",
                            @kIOReturnUnsupportedMode:  @"unsupported mode",
                            @kIOReturnUnderrun:         @"data underrun",
                            @kIOReturnOverrun:          @"data overrun",
                            @kIOReturnDeviceError:      @"device error",
                            @kIOReturnNoCompletion:     @"no completion routine",
                            @kIOReturnAborted:          @"operation was aborted",
                            @kIOReturnNoBandwidth:      @"bus bandwidth would be exceeded",
                            @kIOReturnNotResponding:    @"device is not responding",
                            @kIOReturnInvalid:          @"unanticipated driver error",
                            @0:                         @"0"};
    });

    return descriptionMap[[NSNumber numberWithInt:code]];
}

@end
