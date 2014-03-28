//
//  main.m
//  HWMonitorHelper
//
//  Created by Kozlek on 22/02/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

// Based on code from https://github.com/atnan/SMJobBlessXPC

/*

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 Copyright (C) 2011 Apple Inc. All Rights Reserved.


 */

#import <Foundation/Foundation.h>

#import "SmcHelper.h"
#import "smc.h"

#include <xpc/xpc.h>

int main(int argc, const char *argv[])
{
    xpc_connection_t service = xpc_connection_create_mach_service("org.hwsensors.HWMonitorHelper",
                                                                  dispatch_get_main_queue(),
                                                                  XPC_CONNECTION_MACH_SERVICE_LISTENER);

    if (!service)
        exit(EXIT_FAILURE);

    xpc_connection_set_event_handler(service, ^(xpc_object_t connection) {
        xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
            
            xpc_type_t type = xpc_get_type(event);

            if (type == XPC_TYPE_ERROR) {
                if (event == XPC_ERROR_CONNECTION_INVALID) {
                    // The client process on the other end of the connection has either
                    // crashed or cancelled the connection. After receiving this error,
                    // the connection is in an invalid state, and you do not need to
                    // call xpc_connection_cancel(). Just tear down any associated state
                    // here.

                } else if (event == XPC_ERROR_TERMINATION_IMMINENT) {
                    // Handle per-connection termination cleanup.
                }
            }
            else if (type == XPC_TYPE_DICTIONARY) {
                if (xpc_dictionary_get_count(event)) {

                    xpc_connection_t remote = xpc_dictionary_get_remote_connection(event);
                    xpc_object_t reply = xpc_dictionary_create_reply(event);

                    switch (xpc_dictionary_get_int64(event, "command")) {
                            // 1 - Write Number
                        case 1: {
                            const char* key = xpc_dictionary_get_string(event, "key");
                            int64_t value = xpc_dictionary_get_int64(event, "value");

                            io_connect_t smc_connection;

                            if (kIOReturnSuccess == SMCOpen("AppleSMC", &smc_connection)) {
                                [SmcHelper writeKey:[NSString stringWithFormat:@"%s", key]
                                              value:[NSNumber numberWithLongLong:value]
                                         connection:smc_connection];
                                SMCClose(smc_connection);
                                xpc_dictionary_set_int64(reply, "result", kIOReturnSuccess);
                            }
                            else {
                                xpc_dictionary_set_int64(reply, "result", kIOReturnError);
                            }

                            break;
                        }

                        default:
                            xpc_dictionary_set_int64(reply, "result", kIOReturnBadArgument);
                            break;
                    }
                    
                    xpc_connection_send_message(remote, reply);
                    xpc_release(reply);
                }
            }
        });
        
        xpc_connection_resume(connection);

    });
    
    xpc_connection_resume(service);
    
    dispatch_main();

    xpc_release(service);
    
    return EXIT_SUCCESS;
}

