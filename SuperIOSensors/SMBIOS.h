/*
 * Copyright (c) 1998-2009 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
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

#ifndef _APPLESMBIOS_SMBIOS_H
#define _APPLESMBIOS_SMBIOS_H

#include <IOKit/IOLib.h>

//
// Based on System Management BIOS Reference Specification v2.5
//

typedef UInt8  SMBString;
typedef UInt8  SMBByte;
typedef UInt16 SMBWord;
typedef UInt32 SMBDWord;
typedef UInt64 SMBQWord;

#pragma pack(push, 1)             // enable 8-bit struct packing

struct DMIEntryPoint {
    SMBByte    anchor[5];
    SMBByte    checksum;
    SMBWord    tableLength;
    SMBDWord   tableAddress;
    SMBWord    structureCount;
    SMBByte    bcdRevision;
};

struct SMBEntryPoint {
    SMBByte    anchor[4];
    SMBByte    checksum;
    SMBByte    entryPointLength;
    SMBByte    majorVersion;
    SMBByte    minorVersion;
    SMBWord    maxStructureSize;
    SMBByte    entryPointRevision;
    SMBByte    formattedArea[5];
    struct DMIEntryPoint dmi;
};

//
// Header common to all SMBIOS structures
//

struct SMBStructHeader {
    SMBByte    type;
    SMBByte    length;
    SMBWord    handle;
};

#define SMB_STRUCT_HEADER  SMBStructHeader header;

struct SMBAnchor
{
	const SMBStructHeader *	header;
	const UInt8 *			next;
	const UInt8 *			end;
};

#define SMB_ANCHOR_IS_VALID(x)	\
	((x) && ((x)->header) && ((x)->next) && ((x)->end))

#define SMB_ANCHOR_RESET(x)		\
	bzero(x, sizeof(struct SMBAnchor));

//
// SMBIOS structure types.
//

enum {
    kSMBTypeBIOSInformation             =  0,
    kSMBTypeSystemInformation           =  1,
    kSMBTypeBaseBoard					=  2,
    kSMBTypeSystemEnclosure             =  3,
    kSMBTypeProcessorInformation        =  4,
    kSMBTypeMemoryModule                =  6,
    kSMBTypeCacheInformation            =  7,
    kSMBTypeSystemSlot                  =  9,
    kSMBTypePhysicalMemoryArray         = 16,
    kSMBTypeMemoryDevice                = 17,
    kSMBType32BitMemoryErrorInfo        = 18,
    kSMBType64BitMemoryErrorInfo        = 33,

    /* Apple Specific Structures */
    kSMBTypeFirmwareVolume              = 128,
    kSMBTypeMemorySPD                   = 130,
    kSMBTypeOemProcessorType            = 131,
    kSMBTypeOemProcessorBusSpeed        = 132
};

//
// BIOS Information (Type 0)
//
struct SMBBIOSInformation {
    SMB_STRUCT_HEADER               // Type 0
    SMBString  vendor;              // BIOS vendor name
    SMBString  version;             // BIOS version
    SMBWord    startSegment;        // BIOS segment start
    SMBString  releaseDate;         // BIOS release date
    SMBByte    romSize;             // (n); 64K * (n+1) bytes
    SMBQWord   characteristics;     // supported BIOS functions
};

//
// System Information (Type 1)
//

struct SMBSystemInformation {
    // 2.0+ spec (8 bytes)
    SMB_STRUCT_HEADER               // Type 1
    SMBString  manufacturer;
    SMBString  productName;
    SMBString  version;
    SMBString  serialNumber;
    // 2.1+ spec (25 bytes)
    SMBByte    uuid[16];            // can be all 0 or all 1's
    SMBByte    wakeupReason;        // reason for system wakeup
};

//
// Base Board (Type 2)
//

struct SMBBaseBoard {
    SMB_STRUCT_HEADER               // Type 2
    SMBString	manufacturer;
    SMBString	product;
    SMBString	version;
    SMBString	serialNumber;
    SMBString	assetTagNumber;
    SMBByte		featureFlags;
    SMBString	locationInChassis;
    SMBWord		chassisHandle;
    SMBByte		boardType;
    SMBByte		numberOfContainedHandles;
	// 0 - 255 contained handles go here but we do not include
	// them in our structure. Be careful to use numberOfContainedHandles
	// times sizeof(SMBWord) when computing the actual record size,
	// if you need it.
};

// Values for boardType in Type 2 records
enum {
    kSMBBaseBoardUnknown				= 0x01,
    kSMBBaseBoardOther					= 0x02,
    kSMBBaseBoardServerBlade			= 0x03,
    kSMBBaseBoardConnectivitySwitch		= 0x04,
    kSMBBaseBoardSystemMgmtModule		= 0x05,
    kSMBBaseBoardProcessorModule		= 0x06,
    kSMBBaseBoardIOModule				= 0x07,
    kSMBBaseBoardMemoryModule			= 0x08,
    kSMBBaseBoardDaughter				= 0x09,
    kSMBBaseBoardMotherboard			= 0x0A,
    kSMBBaseBoardProcessorMemoryModule	= 0x0B,
    kSMBBaseBoardProcessorIOModule		= 0x0C,
    kSMBBaseBoardInterconnect			= 0x0D,
};


//
// System Enclosure (Type 3)
//

struct SMBSystemEnclosure {
    SMB_STRUCT_HEADER               // Type 3
    SMBString  manufacturer;
    SMBByte    type;
    SMBString  version;
    SMBString  serialNumber;
    SMBString  assetTagNumber;
    SMBByte    bootupState;
    SMBByte    powerSupplyState;
    SMBByte    thermalState;
    SMBByte    securityStatus;
    SMBDWord   oemDefined;
};

//
// Processor Information (Type 4)
//

struct SMBProcessorInformation {
    // 2.0+ spec (26 bytes)
    SMB_STRUCT_HEADER               // Type 4
    SMBString  socketDesignation;
    SMBByte    processorType;       // CPU = 3
    SMBByte    processorFamily;     // processor family enum
    SMBString  manufacturer;
    SMBQWord   processorID;         // based on CPUID
    SMBString  processorVersion;
    SMBByte    voltage;             // bit7 cleared indicate legacy mode
    SMBWord    externalClock;       // external clock in MHz
    SMBWord    maximumClock;        // max internal clock in MHz
    SMBWord    currentClock;        // current internal clock in MHz
    SMBByte    status;
    SMBByte    processorUpgrade;    // processor upgrade enum
    // 2.1+ spec (32 bytes)
    SMBWord    L1CacheHandle;
    SMBWord    L2CacheHandle;
    SMBWord    L3CacheHandle;
    // 2.3+ spec (35 bytes)
    SMBString  serialNumber;
    SMBString  assetTag;
    SMBString  partNumber;
};

#define kSMBProcessorInformationMinSize     26

//
// Memory Module Information (Type 6)
// Obsoleted since SMBIOS version 2.1
//

struct SMBMemoryModule {
    SMB_STRUCT_HEADER               // Type 6
    SMBString  socketDesignation;
    SMBByte    bankConnections;
    SMBByte    currentSpeed;
    SMBWord    currentMemoryType;
    SMBByte    installedSize;
    SMBByte    enabledSize;
    SMBByte    errorStatus;
};

#define kSMBMemoryModuleSizeNotDeterminable 0x7D
#define kSMBMemoryModuleSizeNotEnabled      0x7E
#define kSMBMemoryModuleSizeNotInstalled    0x7F

//
// Cache Information (Type 7)
//

struct SMBCacheInformation {
    SMB_STRUCT_HEADER               // Type 7
    SMBString  socketDesignation;
    SMBWord    cacheConfiguration;
    SMBWord    maximumCacheSize;
    SMBWord    installedSize;
    SMBWord    supportedSRAMType;
    SMBWord    currentSRAMType;
    SMBByte    cacheSpeed;
    SMBByte    errorCorrectionType;
    SMBByte    systemCacheType;
    SMBByte    associativity;
};

struct SMBSystemSlot {
    // 2.0+ spec (12 bytes)
    SMB_STRUCT_HEADER               // Type 9
    SMBString   slotDesignation;
    SMBByte     slotType;
    SMBByte     slotDataBusWidth;
    SMBByte     currentUsage;
    SMBByte     slotLength;
    SMBWord     slotID;
    SMBByte     slotCharacteristics1;
    // 2.1+ spec (13 bytes)
    SMBByte     slotCharacteristics2;
};

//
// Physical Memory Array (Type 16)
//

struct SMBPhysicalMemoryArray {
    // 2.1+ spec (15 bytes)
    SMB_STRUCT_HEADER               // Type 16
    SMBByte    physicalLocation;    // physical location
    SMBByte    arrayUse;            // the use for the memory array
    SMBByte    errorCorrection;     // error correction/detection method
    SMBDWord   maximumCapacity;     // maximum memory capacity in kilobytes
    SMBWord    errorHandle;         // handle of a previously detected error
    SMBWord    numMemoryDevices;    // number of memory slots or sockets
};

// Memory Array - Use
enum {
    kSMBMemoryArrayUseOther             = 0x01,
    kSMBMemoryArrayUseUnknown           = 0x02,
    kSMBMemoryArrayUseSystemMemory      = 0x03,
    kSMBMemoryArrayUseVideoMemory       = 0x04,
    kSMBMemoryArrayUseFlashMemory       = 0x05,
    kSMBMemoryArrayUseNonVolatileMemory = 0x06,
    kSMBMemoryArrayUseCacheMemory       = 0x07
};

// Memory Array - Error Correction Types
enum {
    kSMBMemoryArrayErrorCorrectionTypeOther         = 0x01,
    kSMBMemoryArrayErrorCorrectionTypeUnknown       = 0x02,
    kSMBMemoryArrayErrorCorrectionTypeNone          = 0x03,
    kSMBMemoryArrayErrorCorrectionTypeParity        = 0x04,
    kSMBMemoryArrayErrorCorrectionTypeSingleBitECC  = 0x05,
    kSMBMemoryArrayErrorCorrectionTypeMultiBitECC   = 0x06,
    kSMBMemoryArrayErrorCorrectionTypeCRC           = 0x07
};

//
// Memory Device (Type 17)
//

struct SMBMemoryDevice {
    // 2.1+ spec (21 bytes)
    SMB_STRUCT_HEADER               // Type 17
    SMBWord    arrayHandle;         // handle of the parent memory array
    SMBWord    errorHandle;         // handle of a previously detected error
    SMBWord    totalWidth;          // total width in bits; including ECC bits
    SMBWord    dataWidth;           // data width in bits
    SMBWord    memorySize;          // bit15 is scale, 0 = MB, 1 = KB
    SMBByte    formFactor;          // memory device form factor
    SMBByte    deviceSet;           // parent set of identical memory devices
    SMBString  deviceLocator;       // labeled socket; e.g. "SIMM 3"
    SMBString  bankLocator;         // labeled bank; e.g. "Bank 0" or "A"
    SMBByte    memoryType;          // type of memory
    SMBWord    memoryTypeDetail;    // additional detail on memory type
    // 2.3+ spec (27 bytes)
    SMBWord    memorySpeed;         // speed of device in MHz (0 for unknown)
    SMBString  manufacturer;
    SMBString  serialNumber;
    SMBString  assetTag;
    SMBString  partNumber;
};

//
// Firmware Volume Description (Apple Specific - Type 128)
//

enum {
    FW_REGION_RESERVED   = 0,
    FW_REGION_RECOVERY   = 1,
    FW_REGION_MAIN       = 2,
    FW_REGION_NVRAM      = 3,
    FW_REGION_CONFIG     = 4,
    FW_REGION_DIAGVAULT  = 5,

    NUM_FLASHMAP_ENTRIES = 8
};

struct FW_REGION_INFO
{
    SMBDWord   StartAddress;
    SMBDWord   EndAddress;
};

struct SMBFirmwareVolume {
    SMB_STRUCT_HEADER               // Type 128
    SMBByte           RegionCount;
    SMBByte           Reserved[3];
    SMBDWord          FirmwareFeatures;
    SMBDWord          FirmwareFeaturesMask;
    SMBByte           RegionType[ NUM_FLASHMAP_ENTRIES ];
    FW_REGION_INFO    FlashMap[   NUM_FLASHMAP_ENTRIES ];
};

//
// Memory SPD Data   (Apple Specific - Type 130)
//

struct SMBMemorySPD {
	SMB_STRUCT_HEADER               // Type 130
	SMBWord           Type17Handle;
	SMBWord           Offset;
	SMBWord           Size;
	SMBWord           Data[];
};

static const char *
SMBMemoryDeviceTypes[] =
{
    "RAM",          /* 00h  Undefined */
    "RAM",          /* 01h  Other */
    "RAM",          /* 02h  Unknown */
    "DRAM",         /* 03h  DRAM */
    "EDRAM",        /* 04h  EDRAM */
    "VRAM",         /* 05h  VRAM */
    "SRAM",         /* 06h  SRAM */
    "RAM",          /* 07h  RAM */
    "ROM",          /* 08h  ROM */
    "FLASH",        /* 09h  FLASH */
    "EEPROM",       /* 0Ah  EEPROM */
    "FEPROM",       /* 0Bh  FEPROM */
    "EPROM",        /* 0Ch  EPROM */
    "CDRAM",        /* 0Dh  CDRAM */
    "3DRAM",        /* 0Eh  3DRAM */
    "SDRAM",        /* 0Fh  SDRAM */
    "SGRAM",        /* 10h  SGRAM */
    "RDRAM",        /* 11h  RDRAM */
    "DDR SDRAM",    /* 12h  DDR */
    "DDR2 SDRAM",   /* 13h  DDR2 */
    "DDR2 FB-DIMM", /* 14h  DDR2 FB-DIMM */
    "RAM",			/* 15h  unused */
    "RAM",			/* 16h  unused */
    "RAM",			/* 17h  unused */
    "DDR3",			/* 18h  DDR3, chosen in [5776134] */
};

static const int
kSMBMemoryDeviceTypeCount = sizeof(SMBMemoryDeviceTypes)   /
                            sizeof(SMBMemoryDeviceTypes[0]);

//
// OEM Processor Type (Apple Specific - Type 131)
//

struct SMBOemProcessorType {
	SMB_STRUCT_HEADER
	SMBWord    ProcessorType;
};

//
// OEM Processor Bus Speed (Apple Specific - Type 132)
//
struct SMBOemProcessorBusSpeed {
	SMB_STRUCT_HEADER
	SMBWord    ProcessorBusSpeed;   // MT/s unit
};

#pragma pack(pop) // reset to default struct packing

#endif /* !_APPLESMBIOS_SMBIOS_H */
