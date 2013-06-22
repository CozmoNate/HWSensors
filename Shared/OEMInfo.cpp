/*
 * Copyright (c) 1998-2009 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 2.0 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "OemInfo.h"
#include "SMBIOS.h"

#define SIGNATURE_16(A,B)             ((A) | (B<<8))
#define SIGNATURE_32(A,B,C,D)         (SIGNATURE_16(A,B)     | (SIGNATURE_16(C,D)     << 16))
#define SIGNATURE_64(A,B,C,D,E,F,G,H) (SIGNATURE_32(A,B,C,D) | ((UINT64)(SIGNATURE_32(E,F,G,H)) << 32))

#define SMBIOS_PTR        SIGNATURE_32('_','S','M','_')

//---------------------------------------------------------------------------
// SMBPackedStrings class

class SMBPackedStrings 
{
protected:
    const char * _start;
    const char * _limit;
    
public:
    SMBPackedStrings( const SMBStructHeader * header, const void * limit );
    SMBPackedStrings( const SMBStructHeader * header );
    
    const char * stringAtIndex( UInt8 index, UInt8 * length = 0 ) const;
    
    void setDataProperty( IORegistryEntry * entry,
                         const char *      key,
                         UInt8             index ) const;
    
    void setStringProperty( IORegistryEntry * entry,
                           const char *      key,
                           UInt8             index ) const;
};

SMBPackedStrings::SMBPackedStrings( const SMBStructHeader * header,
                                   const void *            limit )
{
    _start = (const char *) header + header->length;
    _limit = (const char *) limit;
}

SMBPackedStrings::SMBPackedStrings( const SMBStructHeader * header )
{
    _start = (const char *) header + header->length;
	
	const char * cp = _start;
	// Find the double null at the end of the record
	while( cp[0] || cp[1]) cp++;
	
	_limit = &cp[1];
}

const char * SMBPackedStrings::stringAtIndex( UInt8 index, UInt8 * length ) const
{
    const char * last = 0;
    const char * next = _start;
    
    if ( length ) *length = 0;
    
    while ( index-- )
    {
        last = 0;
        for ( const char * cp = next; cp < _limit; cp++ )
        {
            if ( *cp == '\0' )
            {
                last = next;
                next = cp + 1;
                break;
            }
        }
        if ( last == 0 ) break;
    }
    
    if ( last )
    {
        while (*last == ' ') last++;
        if (length)
        {
            UInt8 len;
            for ( len = next - last - 1; len && last[len - 1] == ' '; len-- )
                ;
            *length = len; // number of chars not counting the terminating NULL
        }
    }
    
    return last ? last : "";
}

void SMBPackedStrings::setDataProperty( IORegistryEntry * entry,
                                       const char *      key,
                                       UInt8             index ) const
{
    UInt8        length;
    const char * string = SMBPackedStrings::stringAtIndex(index, &length);
    
    if (length)
    {
        OSData * data = OSData::withCapacity(length + 1);
        if (data)
        {
            data->appendBytes(string, length);
            data->appendByte('\0', 1);
            entry->setProperty(key, data);
            OSSafeRelease(data);
        }
    }
}

void SMBPackedStrings::setStringProperty( IORegistryEntry * entry,
                                         const char *      key,
                                         UInt8             index ) const
{
    UInt8        length;
    const char * string = SMBPackedStrings::stringAtIndex(index, &length);
    
    if (length)
    {
        OSString * strObj = OSString::withCString(string);
        if (strObj)
        {
            entry->setProperty(key, strObj);
            OSSafeRelease(strObj);
        }
    }
}

//---------------------------------------------------------------------------

static UInt8 checksum8( void * start, UInt length )
{
    UInt8   csum = 0;
    UInt8 * cp = (UInt8 *) start;
    
    for (UInt i = 0; i < length; i++)
        csum += *cp++;
    
    return csum;
}

OSString* getManufacturerNameFromOEMName(OSString *name)
{
    if (!name) {
        return NULL;
    }
    
    OSString *manufacturer = NULL;
    
    if (name->isEqualTo("Apple Inc.")) manufacturer = OSString::withCString("Apple");
    if (name->isEqualTo("ASUSTeK Computer INC.") ||
        name->isEqualTo("ASUSTeK COMPUTER INC.")) manufacturer = OSString::withCString("ASUS");
    if (name->isEqualTo("Dell Inc.")) manufacturer = OSString::withCString("Dell");
    if (name->isEqualTo("DFI") || name->isEqualTo("DFI Inc.")) manufacturer = OSString::withCString("DFI");
    if (name->isEqualTo("EPoX COMPUTER CO., LTD")) manufacturer = OSString::withCString("EPoX");
    if (name->isEqualTo("First International Computer, Inc.")) manufacturer = OSString::withCString("FIC");
    if (name->isEqualTo("FUJITSU") ||
        name->isEqualTo("FUJITSU SIEMENS")) manufacturer = OSString::withCString("FUJITSU");
    if (name->isEqualTo("Gigabyte Technology Co., Ltd.")) manufacturer = OSString::withCString("Gigabyte");
    if (name->isEqualTo("Hewlett-Packard")) manufacturer = OSString::withCString("HP");
    if (name->isEqualTo("IBM")) manufacturer = OSString::withCString("IBM");
    if (name->isEqualTo("Intel") ||
        name->isEqualTo("Intel Corp.") ||
        name->isEqualTo("Intel Corporation")||
        name->isEqualTo("INTEL Corporation")) manufacturer = OSString::withCString("Intel");
    if (name->isEqualTo("Lenovo") || name->isEqualTo("LENOVO")) manufacturer = OSString::withCString("Lenovo");
    if (name->isEqualTo("Micro-Star International") ||
        name->isEqualTo("MICRO-STAR INTERNATIONAL CO., LTD") ||
        name->isEqualTo("MICRO-STAR INTERNATIONAL CO.,LTD") ||
        name->isEqualTo("MSI")) manufacturer = OSString::withCString("MSI");
    
    if (!manufacturer && !name->isEqualTo("To be filled by O.E.M."))
        manufacturer = OSString::withString(name);
    
    return manufacturer;
}

static void processSMBIOSStructureType2(IOService *provider, const SMBBaseBoard *baseBoard, SMBPackedStrings *strings)
{
    if (baseBoard->header.length < 8)
        return;
    
    OSString *manufacturer = NULL;
    
    if (OSString *name = OSString::withCString(strings->stringAtIndex(baseBoard->manufacturer))) {
        manufacturer = getManufacturerNameFromOEMName(name);
        OSSafeRelease(name);
    }
    
    if (manufacturer) {
        provider->setProperty(kOEMInfoManufacturer, manufacturer);
        OSSafeRelease(manufacturer);
    }
    else strings->setStringProperty(provider, kOEMInfoManufacturer,  baseBoard->manufacturer);
    
    strings->setStringProperty(provider, kOEMInfoProduct, baseBoard->product);
}

static void decodeSMBIOSStructure(IOService *provider, const SMBStructHeader *structureHeader, const void *tableBoundary)
{
    const union SMBStructUnion {
        SMBBIOSInformation      bios;
        SMBSystemInformation    system;
		SMBBaseBoard			baseBoard;
        SMBMemoryModule         memoryModule;
        SMBSystemSlot           slot;
        SMBPhysicalMemoryArray  memoryArray;
        SMBMemoryDevice         memoryDevice;
		SMBFirmwareVolume       fv;
		SMBMemorySPD            spd;
    } * u = (const SMBStructUnion *) structureHeader;
    
    SMBPackedStrings strings = SMBPackedStrings( structureHeader,
                                                tableBoundary );
    
    switch ( structureHeader->type )
    {
        case kSMBTypeBIOSInformation:
            //processSMBIOSStructureType0( &u->bios, &strings );
            break;
            
        case kSMBTypeSystemInformation:
            //processSMBIOSStructureType1( &u->system, &strings );
            break;
            
		case kSMBTypeBaseBoard:
			processSMBIOSStructureType2(provider, &u->baseBoard, &strings );
			break;
			
        case kSMBTypeMemoryModule:
            //            processSMBIOSStructureType6( &u->memoryModule, &strings );
            break;
            
        case kSMBTypeSystemSlot:
            //            processSMBIOSStructureType9( &u->slot, &strings );
            break;
            
        case kSMBTypePhysicalMemoryArray:
            //            processSMBIOSStructureType16( &u->memoryArray, &strings );
            break;
            
        case kSMBTypeMemoryDevice:
            //            processSMBIOSStructureType17( &u->memoryDevice, &strings );
            break;
            
        case kSMBTypeFirmwareVolume:
            //			processSMBIOSStructureType128( &u->fv, &strings );
			break;
            
		case kSMBTypeMemorySPD:
            //			processSMBIOSStructureType130( &u->spd, &strings );
			break;
    }
}

static void decodeSMBIOSTable(IOService *provider, const void *tableData, UInt16 tableLength, UInt16 structureCount)
{
    const SMBStructHeader * header;
    const UInt8 *           next = (const UInt8 *) tableData;
    const UInt8 *           end  = next + tableLength;
    
    while ( structureCount-- && (end > next + sizeof(SMBStructHeader)) )
    {
        header = (const SMBStructHeader *) next;
        if (header->length > end - next) break;
        
        decodeSMBIOSStructure(provider, header, end );
        
        // Skip the formatted area of the structure.
        
        next += header->length;
        
        // Skip the unformatted structure area at the end (strings).
        // Look for a terminating double NULL.
        
        for ( ; end > next + sizeof(SMBStructHeader); next++ )
        {
            if ( next[0] == 0 && next[1] == 0 )
            {
                next += 2; break;
            }
        }
    }
}

bool setOemProperties(IOService *provider)
{
    SMBEntryPoint* eps = 0;
	IOMemoryDescriptor* dmiMemory = 0;
	IOItemCount dmiStructureCount = 0;
    
  	UInt8* biosAddress = NULL;	
    
    IOMemoryDescriptor * biosMemory = 0;
    IOMemoryMap * biosMap = 0;
    
    biosMemory = IOMemoryDescriptor::withPhysicalAddress( 0xf0000,0xfffff-0xf0000+1,kIODirectionOutIn);
    
    if(biosMemory)
    {
        biosMap = biosMemory->map();
        
        if(biosMap)
        {
            biosAddress = (UInt8 *) biosMap->getVirtualAddress();
        }
    }
    
    
	// Search 0x0f0000 - 0x0fffff for SMBIOS Ptr
	if(biosAddress) {
        for (UInt32 Address = 0; Address < biosMap->getLength(); Address += 0x10) {
            if (*(UInt32 *)(biosAddress + Address) == SMBIOS_PTR) {
                eps = (SMBEntryPoint *)(biosAddress + Address);
                continue;
            }
        }
    }
    
    if(eps) {
        if (memcmp(eps->anchor, "_SM_", 4) == 0) {
            UInt8 csum;
            
            csum = checksum8(eps, sizeof(SMBEntryPoint));
            
            /*HWSensorsDebugLog("DMI checksum       = 0x%x", csum);
             HWSensorsDebugLog("DMI tableLength    = %d",
             eps->dmi.tableLength);
             HWSensorsDebugLog("DMI tableAddress   = 0x%x",
             (uint32_t) eps->dmi.tableAddress);
             HWSensorsDebugLog("DMI structureCount = %d",
             eps->dmi.structureCount);
             HWSensorsDebugLog("DMI bcdRevision    = %x",
             eps->dmi.bcdRevision);*/
            
            if (csum == 0 && eps->dmi.tableLength && eps->dmi.structureCount) {
                dmiStructureCount = eps->dmi.structureCount;
                dmiMemory = IOMemoryDescriptor::withPhysicalAddress(eps->dmi.tableAddress, eps->dmi.tableLength,kIODirectionOutIn );
            }
            /*else
             {
             HWSensorsDebugLog("no DMI structure found");
             }*/
        }
    }
    
    if (biosMap)
        OSSafeReleaseNULL(biosMap);
    
    if(biosMemory)
        OSSafeReleaseNULL(biosMemory);
    
    if ( dmiMemory ) {
        if (IOMemoryMap *fDMIMemoryMap = dmiMemory->map())        {
            decodeSMBIOSTable(provider, (void *) fDMIMemoryMap->getVirtualAddress(), fDMIMemoryMap->getLength(), dmiStructureCount );
        
            OSSafeReleaseNULL(fDMIMemoryMap);
        }
        
        OSSafeReleaseNULL(dmiMemory);
    }
    
    return true;
}
