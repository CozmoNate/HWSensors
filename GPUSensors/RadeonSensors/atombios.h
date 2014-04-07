//
//  atombios.h
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

/*
 * Copyright 2006-2007 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef HWSensors_atombios_h
#define HWSensors_atombios_h

typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;

/* Common header for all ROM Data tables.
 Every table pointed  _ATOM_MASTER_DATA_TABLE has this common header.
 And the pointer actually points to this header. */

typedef struct _ATOM_COMMON_TABLE_HEADER
{
    USHORT usStructureSize;
    UCHAR  ucTableFormatRevision;   /*Change it when the Parser is not backward compatible */
    UCHAR  ucTableContentRevision;  /*Change it only when the table needs to change but the firmware */
    /*Image can't be updated, while Driver needs to carry the new table! */
}ATOM_COMMON_TABLE_HEADER;

/****************************************************************************/
// Structure used in Data.mtb
/****************************************************************************/
typedef struct _ATOM_MASTER_LIST_OF_DATA_TABLES
{
    USHORT        UtilityPipeLine;	        // Offest for the utility to get parser info,Don't change this position!
    USHORT        MultimediaCapabilityInfo; // Only used by MM Lib,latest version 1.1, not configuable from Bios, need to include the table to build Bios
    USHORT        MultimediaConfigInfo;     // Only used by MM Lib,latest version 2.1, not configuable from Bios, need to include the table to build Bios
    USHORT        StandardVESA_Timing;      // Only used by Bios
    USHORT        FirmwareInfo;             // Shared by various SW components,latest version 1.4
    USHORT        PaletteData;              // Only used by BIOS
    USHORT        LCD_Info;                 // Shared by various SW components,latest version 1.3, was called LVDS_Info
    USHORT        DIGTransmitterInfo;       // Internal used by VBIOS only version 3.1
    USHORT        AnalogTV_Info;            // Shared by various SW components,latest version 1.1
    USHORT        SupportedDevicesInfo;     // Will be obsolete from R600
    USHORT        GPIO_I2C_Info;            // Shared by various SW components,latest version 1.2 will be used from R600
    USHORT        VRAM_UsageByFirmware;     // Shared by various SW components,latest version 1.3 will be used from R600
    USHORT        GPIO_Pin_LUT;             // Shared by various SW components,latest version 1.1
    USHORT        VESA_ToInternalModeLUT;   // Only used by Bios
    USHORT        ComponentVideoInfo;       // Shared by various SW components,latest version 2.1 will be used from R600
    USHORT        PowerPlayInfo;            // Shared by various SW components,latest version 2.1,new design from R600
    USHORT        CompassionateData;        // Will be obsolete from R600
    USHORT        SaveRestoreInfo;          // Only used by Bios
    USHORT        PPLL_SS_Info;             // Shared by various SW components,latest version 1.2, used to call SS_Info, change to new name because of int ASIC SS info
    USHORT        OemInfo;                  // Defined and used by external SW, should be obsolete soon
    USHORT        XTMDS_Info;               // Will be obsolete from R600
    USHORT        MclkSS_Info;              // Shared by various SW components,latest version 1.1, only enabled when ext SS chip is used
    USHORT        Object_Header;            // Shared by various SW components,latest version 1.1
    USHORT        IndirectIOAccess;         // Only used by Bios,this table position can't change at all!!
    USHORT        MC_InitParameter;         // Only used by command table
    USHORT        ASIC_VDDC_Info;						// Will be obsolete from R600
    USHORT        ASIC_InternalSS_Info;			// New tabel name from R600, used to be called "ASIC_MVDDC_Info"
    USHORT        TV_VideoMode;							// Only used by command table
    USHORT        VRAM_Info;								// Only used by command table, latest version 1.3
    USHORT        MemoryTrainingInfo;				// Used for VBIOS and Diag utility for memory training purpose since R600. the new table rev start from 2.1
    USHORT        IntegratedSystemInfo;			// Shared by various SW components
    USHORT        ASIC_ProfilingInfo;				// New table name from R600, used to be called "ASIC_VDDCI_Info" for pre-R600
    USHORT        VoltageObjectInfo;				// Shared by various SW components, latest version 1.1
	USHORT				PowerSourceInfo;					// Shared by various SW components, latest versoin 1.1
}ATOM_MASTER_LIST_OF_DATA_TABLES;


// Macros used by driver
#ifdef __cplusplus
#define GetIndexIntoMasterTable(MasterOrData, FieldName) ((reinterpret_cast<char*>(&(static_cast<ATOM_MASTER_LIST_OF_##MasterOrData##_TABLES*>(0))->FieldName)-static_cast<char*>(0))/sizeof(USHORT))

#define GET_COMMAND_TABLE_COMMANDSET_REVISION(TABLE_HEADER_OFFSET) (((static_cast<ATOM_COMMON_TABLE_HEADER*>(TABLE_HEADER_OFFSET))->ucTableFormatRevision )&0x3F)
#define GET_COMMAND_TABLE_PARAMETER_REVISION(TABLE_HEADER_OFFSET)  (((static_cast<ATOM_COMMON_TABLE_HEADER*>(TABLE_HEADER_OFFSET))->ucTableContentRevision)&0x3F)
#else // not __cplusplus
#define	GetIndexIntoMasterTable(MasterOrData, FieldName) (((char*)(&((ATOM_MASTER_LIST_OF_##MasterOrData##_TABLES*)0)->FieldName)-(char*)0)/sizeof(USHORT))

#define GET_COMMAND_TABLE_COMMANDSET_REVISION(TABLE_HEADER_OFFSET) ((((ATOM_COMMON_TABLE_HEADER*)TABLE_HEADER_OFFSET)->ucTableFormatRevision)&0x3F)
#define GET_COMMAND_TABLE_PARAMETER_REVISION(TABLE_HEADER_OFFSET)  ((((ATOM_COMMON_TABLE_HEADER*)TABLE_HEADER_OFFSET)->ucTableContentRevision)&0x3F)
#endif // __cplusplus

/****************************Legacy Power Play Table Definitions **********************/

//Definitions for ulPowerPlayMiscInfo
#define ATOM_PM_MISCINFO_SPLIT_CLOCK                     0x00000000L
#define ATOM_PM_MISCINFO_USING_MCLK_SRC                  0x00000001L
#define ATOM_PM_MISCINFO_USING_SCLK_SRC                  0x00000002L

#define ATOM_PM_MISCINFO_VOLTAGE_DROP_SUPPORT            0x00000004L
#define ATOM_PM_MISCINFO_VOLTAGE_DROP_ACTIVE_HIGH        0x00000008L

#define ATOM_PM_MISCINFO_LOAD_PERFORMANCE_EN             0x00000010L

#define ATOM_PM_MISCINFO_ENGINE_CLOCK_CONTRL_EN          0x00000020L
#define ATOM_PM_MISCINFO_MEMORY_CLOCK_CONTRL_EN          0x00000040L
#define ATOM_PM_MISCINFO_PROGRAM_VOLTAGE                 0x00000080L  //When this bit set, ucVoltageDropIndex is not an index for GPIO pin, but a voltage ID that SW needs program

#define ATOM_PM_MISCINFO_ASIC_REDUCED_SPEED_SCLK_EN      0x00000100L
#define ATOM_PM_MISCINFO_ASIC_DYNAMIC_VOLTAGE_EN         0x00000200L
#define ATOM_PM_MISCINFO_ASIC_SLEEP_MODE_EN              0x00000400L
#define ATOM_PM_MISCINFO_LOAD_BALANCE_EN                 0x00000800L
#define ATOM_PM_MISCINFO_DEFAULT_DC_STATE_ENTRY_TRUE     0x00001000L
#define ATOM_PM_MISCINFO_DEFAULT_LOW_DC_STATE_ENTRY_TRUE 0x00002000L
#define ATOM_PM_MISCINFO_LOW_LCD_REFRESH_RATE            0x00004000L

#define ATOM_PM_MISCINFO_DRIVER_DEFAULT_MODE             0x00008000L
#define ATOM_PM_MISCINFO_OVER_CLOCK_MODE                 0x00010000L
#define ATOM_PM_MISCINFO_OVER_DRIVE_MODE                 0x00020000L
#define ATOM_PM_MISCINFO_POWER_SAVING_MODE               0x00040000L
#define ATOM_PM_MISCINFO_THERMAL_DIODE_MODE              0x00080000L

#define ATOM_PM_MISCINFO_FRAME_MODULATION_MASK           0x00300000L  //0-FM Disable, 1-2 level FM, 2-4 level FM, 3-Reserved
#define ATOM_PM_MISCINFO_FRAME_MODULATION_SHIFT          20

#define ATOM_PM_MISCINFO_DYN_CLK_3D_IDLE                 0x00400000L
#define ATOM_PM_MISCINFO_DYNAMIC_CLOCK_DIVIDER_BY_2      0x00800000L
#define ATOM_PM_MISCINFO_DYNAMIC_CLOCK_DIVIDER_BY_4      0x01000000L
#define ATOM_PM_MISCINFO_DYNAMIC_HDP_BLOCK_EN            0x02000000L  //When set, Dynamic
#define ATOM_PM_MISCINFO_DYNAMIC_MC_HOST_BLOCK_EN        0x04000000L  //When set, Dynamic
#define ATOM_PM_MISCINFO_3D_ACCELERATION_EN              0x08000000L  //When set, This mode is for acceleated 3D mode

#define ATOM_PM_MISCINFO_POWERPLAY_SETTINGS_GROUP_MASK   0x70000000L  //1-Optimal Battery Life Group, 2-High Battery, 3-Balanced, 4-High Performance, 5- Optimal Performance (Default state with Default clocks)
#define ATOM_PM_MISCINFO_POWERPLAY_SETTINGS_GROUP_SHIFT  28
#define ATOM_PM_MISCINFO_ENABLE_BACK_BIAS                0x80000000L

#define ATOM_PM_MISCINFO2_SYSTEM_AC_LITE_MODE            0x00000001L
#define ATOM_PM_MISCINFO2_MULTI_DISPLAY_SUPPORT          0x00000002L
#define ATOM_PM_MISCINFO2_DYNAMIC_BACK_BIAS_EN           0x00000004L
#define ATOM_PM_MISCINFO2_FS3D_OVERDRIVE_INFO            0x00000008L
#define ATOM_PM_MISCINFO2_FORCEDLOWPWR_MODE              0x00000010L
#define ATOM_PM_MISCINFO2_VDDCI_DYNAMIC_VOLTAGE_EN       0x00000020L
#define ATOM_PM_MISCINFO2_VIDEO_PLAYBACK_CAPABLE         0x00000040L  //If this bit is set in multi-pp mode, then driver will pack up one with the minior power consumption.
//If it's not set in any pp mode, driver will use its default logic to pick a pp mode in video playback
#define ATOM_PM_MISCINFO2_NOT_VALID_ON_DC                0x00000080L
#define ATOM_PM_MISCINFO2_STUTTER_MODE_EN                0x00000100L
#define ATOM_PM_MISCINFO2_UVD_SUPPORT_MODE               0x00000200L

//ucTableFormatRevision=1
//ucTableContentRevision=1
typedef struct  _ATOM_POWERMODE_INFO
{
    ULONG     ulMiscInfo;                 //The power level should be arranged in ascending order
    ULONG     ulReserved1;                // must set to 0
    ULONG     ulReserved2;                // must set to 0
    USHORT    usEngineClock;
    USHORT    usMemoryClock;
    UCHAR     ucVoltageDropIndex;         // index to GPIO table
    UCHAR     ucSelectedPanel_RefreshRate;// panel refresh rate
    UCHAR     ucMinTemperature;
    UCHAR     ucMaxTemperature;
    UCHAR     ucNumPciELanes;             // number of PCIE lanes
}ATOM_POWERMODE_INFO;

//ucTableFormatRevision=2
//ucTableContentRevision=1
typedef struct  _ATOM_POWERMODE_INFO_V2
{
    ULONG     ulMiscInfo;                 //The power level should be arranged in ascending order
    ULONG     ulMiscInfo2;
    ULONG     ulEngineClock;
    ULONG     ulMemoryClock;
    UCHAR     ucVoltageDropIndex;         // index to GPIO table
    UCHAR     ucSelectedPanel_RefreshRate;// panel refresh rate
    UCHAR     ucMinTemperature;
    UCHAR     ucMaxTemperature;
    UCHAR     ucNumPciELanes;             // number of PCIE lanes
}ATOM_POWERMODE_INFO_V2;

//ucTableFormatRevision=2
//ucTableContentRevision=2
typedef struct  _ATOM_POWERMODE_INFO_V3
{
    ULONG     ulMiscInfo;                 //The power level should be arranged in ascending order
    ULONG     ulMiscInfo2;
    ULONG     ulEngineClock;
    ULONG     ulMemoryClock;
    UCHAR     ucVoltageDropIndex;         // index to Core (VDDC) votage table
    UCHAR     ucSelectedPanel_RefreshRate;// panel refresh rate
    UCHAR     ucMinTemperature;
    UCHAR     ucMaxTemperature;
    UCHAR     ucNumPciELanes;             // number of PCIE lanes
    UCHAR     ucVDDCI_VoltageDropIndex;   // index to VDDCI votage table
}ATOM_POWERMODE_INFO_V3;


#define ATOM_MAX_NUMBEROF_POWER_BLOCK  8

#define ATOM_PP_OVERDRIVE_INTBITMAP_AUXWIN            0x01
#define ATOM_PP_OVERDRIVE_INTBITMAP_OVERDRIVE         0x02

#define ATOM_PP_OVERDRIVE_THERMALCONTROLLER_LM63      0x01
#define ATOM_PP_OVERDRIVE_THERMALCONTROLLER_ADM1032   0x02
#define ATOM_PP_OVERDRIVE_THERMALCONTROLLER_ADM1030   0x03
#define ATOM_PP_OVERDRIVE_THERMALCONTROLLER_MUA6649   0x04
#define ATOM_PP_OVERDRIVE_THERMALCONTROLLER_LM64      0x05
#define ATOM_PP_OVERDRIVE_THERMALCONTROLLER_F75375    0x06
#define ATOM_PP_OVERDRIVE_THERMALCONTROLLER_ASC7512   0x07	// Andigilog


typedef struct  _ATOM_POWERPLAY_INFO
{
    ATOM_COMMON_TABLE_HEADER	sHeader;
    UCHAR    ucOverdriveThermalController;
    UCHAR    ucOverdriveI2cLine;
    UCHAR    ucOverdriveIntBitmap;
    UCHAR    ucOverdriveControllerAddress;
    UCHAR    ucSizeOfPowerModeEntry;
    UCHAR    ucNumOfPowerModeEntries;
    ATOM_POWERMODE_INFO asPowerPlayInfo[ATOM_MAX_NUMBEROF_POWER_BLOCK];
}ATOM_POWERPLAY_INFO;

typedef struct  _ATOM_POWERPLAY_INFO_V2
{
    ATOM_COMMON_TABLE_HEADER	sHeader;
    UCHAR    ucOverdriveThermalController;
    UCHAR    ucOverdriveI2cLine;
    UCHAR    ucOverdriveIntBitmap;
    UCHAR    ucOverdriveControllerAddress;
    UCHAR    ucSizeOfPowerModeEntry;
    UCHAR    ucNumOfPowerModeEntries;
    ATOM_POWERMODE_INFO_V2 asPowerPlayInfo[ATOM_MAX_NUMBEROF_POWER_BLOCK];
}ATOM_POWERPLAY_INFO_V2;

typedef struct  _ATOM_POWERPLAY_INFO_V3
{
    ATOM_COMMON_TABLE_HEADER	sHeader;
    UCHAR    ucOverdriveThermalController;
    UCHAR    ucOverdriveI2cLine;
    UCHAR    ucOverdriveIntBitmap;
    UCHAR    ucOverdriveControllerAddress;
    UCHAR    ucSizeOfPowerModeEntry;
    UCHAR    ucNumOfPowerModeEntries;
    ATOM_POWERMODE_INFO_V3 asPowerPlayInfo[ATOM_MAX_NUMBEROF_POWER_BLOCK];
}ATOM_POWERPLAY_INFO_V3;


/* New PPlib */
/**************************************************************************/
typedef struct _ATOM_PPLIB_THERMALCONTROLLER

{
    UCHAR ucType;           // one of ATOM_PP_THERMALCONTROLLER_*
    UCHAR ucI2cLine;        // as interpreted by DAL I2C
    UCHAR ucI2cAddress;
    UCHAR ucFanParameters;  // Fan Control Parameters.
    UCHAR ucFanMinRPM;      // Fan Minimum RPM (hundreds) -- for display purposes only.
    UCHAR ucFanMaxRPM;      // Fan Maximum RPM (hundreds) -- for display purposes only.
    UCHAR ucReserved;       // ----
    UCHAR ucFlags;          // to be defined
} ATOM_PPLIB_THERMALCONTROLLER;

#define ATOM_PP_FANPARAMETERS_TACHOMETER_PULSES_PER_REVOLUTION_MASK 0x0f
#define ATOM_PP_FANPARAMETERS_NOFAN                                 0x80    // No fan is connected to this controller.

#define ATOM_PP_THERMALCONTROLLER_NONE      0
#define ATOM_PP_THERMALCONTROLLER_LM63      1  // Not used by PPLib
#define ATOM_PP_THERMALCONTROLLER_ADM1032   2  // Not used by PPLib
#define ATOM_PP_THERMALCONTROLLER_ADM1030   3  // Not used by PPLib
#define ATOM_PP_THERMALCONTROLLER_MUA6649   4  // Not used by PPLib
#define ATOM_PP_THERMALCONTROLLER_LM64      5
#define ATOM_PP_THERMALCONTROLLER_F75375    6  // Not used by PPLib
#define ATOM_PP_THERMALCONTROLLER_RV6xx     7
#define ATOM_PP_THERMALCONTROLLER_RV770     8
#define ATOM_PP_THERMALCONTROLLER_ADT7473   9
#define ATOM_PP_THERMALCONTROLLER_EXTERNAL_GPIO     11
#define ATOM_PP_THERMALCONTROLLER_EVERGREEN 12
#define ATOM_PP_THERMALCONTROLLER_EMC2103   13  /* 0x0D */ // Only fan control will be implemented, do NOT show this in PPGen.
#define ATOM_PP_THERMALCONTROLLER_SUMO      14  /* 0x0E */ // Sumo type, used internally
#define ATOM_PP_THERMALCONTROLLER_NISLANDS  15
#define ATOM_PP_THERMALCONTROLLER_SISLANDS  16
#define ATOM_PP_THERMALCONTROLLER_LM96163   17
#define ATOM_PP_THERMALCONTROLLER_CISLANDS  18
#define ATOM_PP_THERMALCONTROLLER_KAVERI    19

// Thermal controller 'combo type' to use an external controller for Fan control and an internal controller for thermal.
// We probably should reserve the bit 0x80 for this use.
// To keep the number of these types low we should also use the same code for all ASICs (i.e. do not distinguish RV6xx and RV7xx Internal here).
// The driver can pick the correct internal controller based on the ASIC.

#define ATOM_PP_THERMALCONTROLLER_ADT7473_WITH_INTERNAL   0x89    // ADT7473 Fan Control + Internal Thermal Controller
#define ATOM_PP_THERMALCONTROLLER_EMC2103_WITH_INTERNAL   0x8D    // EMC2103 Fan Control + Internal Thermal Controller

//// ATOM_PPLIB_POWERPLAYTABLE::ulPlatformCaps
#define ATOM_PP_PLATFORM_CAP_BACKBIAS 1
#define ATOM_PP_PLATFORM_CAP_POWERPLAY 2
#define ATOM_PP_PLATFORM_CAP_SBIOSPOWERSOURCE 4
#define ATOM_PP_PLATFORM_CAP_ASPM_L0s 8
#define ATOM_PP_PLATFORM_CAP_ASPM_L1 16
#define ATOM_PP_PLATFORM_CAP_HARDWAREDC 32
#define ATOM_PP_PLATFORM_CAP_GEMINIPRIMARY 64
#define ATOM_PP_PLATFORM_CAP_STEPVDDC 128
#define ATOM_PP_PLATFORM_CAP_VOLTAGECONTROL 256
#define ATOM_PP_PLATFORM_CAP_SIDEPORTCONTROL 512
#define ATOM_PP_PLATFORM_CAP_TURNOFFPLL_ASPML1 1024
#define ATOM_PP_PLATFORM_CAP_HTLINKCONTROL 2048
#define ATOM_PP_PLATFORM_CAP_MVDDCONTROL 4096
#define ATOM_PP_PLATFORM_CAP_GOTO_BOOT_ON_ALERT 0x2000              // Go to boot state on alerts, e.g. on an AC->DC transition.
#define ATOM_PP_PLATFORM_CAP_DONT_WAIT_FOR_VBLANK_ON_ALERT 0x4000   // Do NOT wait for VBLANK during an alert (e.g. AC->DC transition).
#define ATOM_PP_PLATFORM_CAP_VDDCI_CONTROL 0x8000                   // Does the driver control VDDCI independently from VDDC.
#define ATOM_PP_PLATFORM_CAP_REGULATOR_HOT 0x00010000               // Enable the 'regulator hot' feature.
#define ATOM_PP_PLATFORM_CAP_BACO          0x00020000               // Does the driver supports BACO state.


typedef struct _ATOM_PPLIB_POWERPLAYTABLE
{
    ATOM_COMMON_TABLE_HEADER sHeader;
    
    UCHAR ucDataRevision;
    
    UCHAR ucNumStates;
    UCHAR ucStateEntrySize;
    UCHAR ucClockInfoSize;
    UCHAR ucNonClockSize;
    
    // offset from start of this table to array of ucNumStates ATOM_PPLIB_STATE structures
    USHORT usStateArrayOffset;
    
    // offset from start of this table to array of ASIC-specific structures,
    // currently ATOM_PPLIB_CLOCK_INFO.
    USHORT usClockInfoArrayOffset;
    
    // offset from start of this table to array of ATOM_PPLIB_NONCLOCK_INFO
    USHORT usNonClockInfoArrayOffset;
    
    USHORT usBackbiasTime;    // in microseconds
    USHORT usVoltageTime;     // in microseconds
    USHORT usTableSize;       //the size of this structure, or the extended structure
    
    ULONG ulPlatformCaps;            // See ATOM_PPLIB_CAPS_*
    
    ATOM_PPLIB_THERMALCONTROLLER    sThermalController;
    
    USHORT usBootClockInfoOffset;
    USHORT usBootNonClockInfoOffset;
    
} ATOM_PPLIB_POWERPLAYTABLE;

typedef struct _ATOM_PPLIB_POWERPLAYTABLE2
{
    ATOM_PPLIB_POWERPLAYTABLE basicTable;
    UCHAR   ucNumCustomThermalPolicy;
    USHORT  usCustomThermalPolicyArrayOffset;
}ATOM_PPLIB_POWERPLAYTABLE2, *LPATOM_PPLIB_POWERPLAYTABLE2;

typedef struct _ATOM_PPLIB_POWERPLAYTABLE3
{
    ATOM_PPLIB_POWERPLAYTABLE2 basicTable2;
    USHORT                     usFormatID;                      // To be used ONLY by PPGen.
    USHORT                     usFanTableOffset;
    USHORT                     usExtendendedHeaderOffset;
} ATOM_PPLIB_POWERPLAYTABLE3, *LPATOM_PPLIB_POWERPLAYTABLE3;

#endif
