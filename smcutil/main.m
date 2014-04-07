//
//  main.m
//  smcutil
//
//  Created by kozlek on 31.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 * Apple System Management Control (SMC) Tool
 * Copyright (C) 2006 devnull
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#import <Foundation/Foundation.h>
#import <stdio.h>

#import "SmcHelper.h"

#define NSStr(x) [NSString stringWithCString:(x) encoding:NSASCIIStringEncoding]

#define OPTION_NONE     0
#define OPTION_LIST     1
#define OPTION_READ     2
#define OPTION_WRITE    3
#define OPTION_HELP     4

void usage(const char* prog)
{
    printf("smcutil v%s\n", VERSION);
    printf("Usage:\n");
    printf("%s [options]\n", prog);
    printf("    -l         : list of all keys\n");
    printf("    -r <key>   : show key value\n");
    printf("    -h         : help\n");
    printf("\n");
}

UInt32 SMCReadIndexCount(io_connect_t connection)
{
    return [SmcHelper readNumericKey:@"#KEY" connection:connection];
}

bool printKeyValue(SMCVal_t val)
{
    if (val.dataSize > 0)
    {
        if (!strncasecmp(val.dataType, "ch8*", 4)) {
            for (int i = 0; i < val.dataSize; i++)
                printf("%c", (char)val.bytes[i]);
        }
        else if (!strncasecmp(val.dataType, "flag", 4)) {
            printf(val.bytes[0] ? "TRUE" : "FALSE");
        }
        else  if ([SmcHelper isValidIntegerSmcType:NSStr(val.dataType)]) {
            printf("%d", [SmcHelper decodeNumericValueFromBuffer:val.bytes length:val.dataSize type:val.dataType].unsignedIntValue);
        }
        else if ([SmcHelper isValidFloatingSmcType:NSStr(val.dataType)]) {
            printf("%.2f", [SmcHelper decodeNumericValueFromBuffer:val.bytes length:val.dataSize type:val.dataType].floatValue);
        }
        else return false;
        
        return true;
    }
    else
    {
        printf("no data");
    }
    
    return false;
}

void printValueBytes(SMCVal_t val)
{
    printf("(bytes");
    if (val.dataSize > 0)
    {
        for (int i = 0; i < val.dataSize; i++)
            printf(" %02x", (unsigned char) val.bytes[i]);
    }
    else {
        printf(" -");
    }
    printf(")");
}

int main(int argc, const char * argv[])
{
    @autoreleasepool {
        int c, option;
        char key[5];
        SMCVal_t val;

        option = OPTION_HELP;
        
        while ((c = getopt(argc, argv, "lr")) != -1)
        {
            switch(c)
            {
                case 'l':
                    option = OPTION_LIST;
                    break;
                case 'r':
                    option = OPTION_READ;
                    break;
                case 'w':
                    option = OPTION_WRITE;
                    break;
                case 'h':
                case '?':
                default:
                    usage(argv[0]);
                    return 1;
            }
        }
        
        io_connect_t connection;
        
        if (kIOReturnSuccess == SMCOpen("AppleSMC", &connection)) {
            
            switch (option) {
                case OPTION_LIST: {
                    
                    UInt32 count = SMCReadIndexCount(connection);
                    
                    for (UInt32 index = 0; index < count; index++) {
                        SMCKeyData_t  inputStructure;
                        SMCKeyData_t  outputStructure;
                        
                        memset(&inputStructure, 0, sizeof(SMCKeyData_t));
                        memset(&outputStructure, 0, sizeof(SMCKeyData_t));
                        memset(&val, 0, sizeof(SMCVal_t));
                        
                        inputStructure.data8 = SMC_CMD_READ_INDEX;
                        inputStructure.data32 = index;
                        
                        if (kIOReturnSuccess == SMCCall(connection, KERNEL_INDEX_SMC, &inputStructure, &outputStructure)) {
                            _ultostr(key, outputStructure.key);
                            
                            if (kIOReturnSuccess == SMCReadKey(connection, key, &val)) {
                                printf("  %-4s  [%-4s]  ", val.key, val.dataType);
                                if (printKeyValue(val))
                                    printf("  ");
                                printValueBytes(val);
                                printf("\n");
                            }
                        }
                    }
                    
                    break;
                }
                    
                case OPTION_READ:
                    snprintf(key, 5, "%s", argv[2]);
                    if (kIOReturnSuccess == SMCReadKey(connection, key, &val)) {
                        printKeyValue(val);
                    }
                    break;

                case OPTION_WRITE: {
                    const char *optarg = argv[3];

                    snprintf(key, 5, "%s", argv[2]);

                    bcopy(val.key, key, 4);

                    int i;
                    char c[3];
                    for (i = 0; i < strlen(optarg); i++)
                    {
                        sprintf(c, "%c%c", optarg[i * 2], optarg[(i * 2) + 1]);
                        val.bytes[i] = (int) strtol(c, NULL, 16);
                    }
                    val.dataSize = i / 2;
                    if ((val.dataSize * 2) != strlen(optarg))
                    {
                        printf("Error: value is not valid\n");
                        return 1;
                    }

                    if (kIOReturnSuccess == SMCWriteKey(connection, &val)) {
                        printKeyValue(val);
                    }
                    break;
                }

                case OPTION_HELP:
                    usage(argv[0]);
                    break;
            }
            
            SMCClose(connection);
        }
        else {
            printf("failed to connect to SMC!\n");
        }
    
    }
    return 0;
}

