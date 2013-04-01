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
#import "HWMonitorSensor.h"
#import "smc.h"

#define NSStr(x) [NSString stringWithCString:(x) encoding:NSASCIIStringEncoding]

#define OPTION_NONE     0
#define OPTION_LIST     1
#define OPTION_READ     2
#define OPTION_HELP     4

void usage(char* prog)
{
    printf("Apple System Management Control (SMC) tool %s\n", VERSION);
    printf("Usage:\n");
    printf("%s [options]\n", prog);
    printf("    -l         : list all keys and values\n");
    printf("    -r <key>   : read key value\n");
    printf("    -h         : help\n");
    printf("\n");
}

UInt32 _strtoul(char *str, int size, int base)
{
    UInt32 total = 0;
    int i;
    
    for (i = 0; i < size; i++)
    {
        if (base == 16)
            total += str[i] << (size - 1 - i) * 8;
        else
            total += (unsigned char) (str[i] << (size - 1 - i) * 8);
    }
    return total;
}

void _ultostr(char *str, UInt32 val)
{
    str[0] = '\0';
    sprintf(str, "%c%c%c%c",
            (unsigned int) val >> 24,
            (unsigned int) val >> 16,
            (unsigned int) val >> 8,
            (unsigned int) val);
}

UInt32 SMCReadIndexCount(io_connect_t connection)
{
    SMCVal_t val;
    
    SMCReadKey(connection, "#KEY", &val);
    return _strtoul(val.bytes, val.dataSize, 10);
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
        else  if ([HWMonitorSensor isValidIntegetType:NSStr(val.dataType)]) {
            printf("%.0f", [HWMonitorSensor decodeNumericData:[NSData dataWithBytes:val.bytes length:val.dataSize] ofType:NSStr(val.dataType)]);
        }
        else if ([HWMonitorSensor isValidFloatingType:NSStr(val.dataType)]) {
            printf("%.2f", [HWMonitorSensor decodeNumericData:[NSData dataWithBytes:val.bytes length:val.dataSize] ofType:NSStr(val.dataType)]);
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
        
        while ((c = getopt(argc, argv, "fhk:lrw:v")) != -1)
        {
            switch(c)
            {
                case 'l':
                    option = OPTION_LIST;
                    break;
                case 'r':
                    option = OPTION_READ;
                    break;
                case 'h':
                case '?':
                default:
                    usage(argv[0]);
                    return 1;
            }
        }
        
        io_connect_t connection;
        
        if (kIOReturnSuccess == SMCOpen(&connection)) {
            
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
                    snprintf(key, 5, argv[3]);
                    if (kIOReturnSuccess == SMCReadKey(connection, key, &val)) {
                        printKeyValue(val);
                    }
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

