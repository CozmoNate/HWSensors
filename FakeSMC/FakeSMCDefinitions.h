//
//  FakeSMCDefinitions.h
//  HWSensors
//
//  Created by mozo on 20.10.11.
//  Copyright (c) 2011 mozodojo. All rights reserved.
//

#ifndef HWSensors_definitions_h
#define HWSensors_definitions_h

// Temperature (*C)
// CPU
#define KEY_FORMAT_CPU_DIE_CORE_TEMPERATURE     "TC%XC" // CPU Core %X Die Digital
#define KEY_FORMAT_CPU_DIODE_TEMPERATURE        "TC%XD" // CPU Core %X Die Analog
#define KEY_CPU_HEATSINK_TEMPERATURE            "Th0H"
#define KEY_CPU_PROXIMITY_TEMPERATURE           "TC0P"
// GPU
#define KEY_FORMAT_GPU_DIODE_TEMPERATURE        "TG%XD"
#define	KEY_FORMAT_GPU_BOARD_TEMPERATURE		"TG%XH"
#define KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE    "TG%XP"
#define KEY_GPU_MEMORY_TEMPERATURE              "TG0M"
// PECI
#define KEY_FORMAT_CPU_PECI_CORE_TEMPERATURE    "TC%Xc" // SNB
#define KEY_PECI_GFX_TEMPERATURE                "TCGc"  // SNB HD2/3000
#define KEY_PECI_SA_TEMPERATURE                 "TCSc"  // SNB 
#define KEY_PECI_PACKAGE_TEMPERATURE            "TCXc"  // SNB 
// NorthBridge, MCH, MCP, PCH
#define KEY_MCH_DIODE_TEMPERATURE               "TN0C"
#define KEY_MCH_HEATSINK_TEMPERATURE            "TN0H"
#define KEY_MCP_DIE_TEMPERATURE                 "TN0D"
#define KEY_MCP_INTERNAL_DIE_TEMPERATURE        "TN1D"
#define KEY_MCP_PROXIMITY_TEMPERATURE           "TM0P" // MCP Proximity/Inlet
#define KEY_NORTHBRIDGE_TEMPERATURE             "TN0P" // MCP Proximity Top Side
#define KEY_NORTHBRIDGE_PROXIMITY_TEMPERATURE   "TN1P"
#define KEY_PCH_DIE_TEMPERATURE                 "TPCD" // SNB PCH Die Digital
#define KEY_PCH_PROXIMITY_TEMPERATURE           "TP0P" // SNB
// Misc
#define KEY_ACDC_TEMPERATURE                    "Tp0C" // PSMI Supply AC/DC Supply 1
#define KEY_AMBIENT_TEMPERATURE                 "TA0P"
#define KEY_AMBIENT1_TEMPERATURE                "TA1P"
#define KEY_DIMM_TEMPERATURE                    "Tm0P" // MLB Proximity/DC In Proximity Airflow
#define KEY_AIRVENT_TEMPERATURE                 "TV0P" // Air Vent Exit
#define KEY_AIRPORT_TEMPERATURE                 "TW0P"

// Voltage (Volts)
// CPU
#define KEY_CPU_VOLTAGE                         "VC0C" // CPU 0 Core
#define KEY_CPU_VOLTAGE_RAW                     "VC0c" // CPU 0 V-Sense
#define KEY_CPU_VCCIO_VOLTAGE                   "VC1C" // CPU VccIO PP1V05
#define KEY_CPU_VCCSA_VOLTAGE                   "VC2C" // CPU VCCSA
#define KEY_CPU_DRAM_VOLTAGE                    "VC5R" // VCSR-DIMM 1.5V S0
#define KEY_CPU_PLL_VOLTAGE                     "VC8R" // CPU 1.8V S0
#define KEY_CPU_VRMSUPPLY0_VOLTAGE              "VS0C" 
#define KEY_CPU_VRMSUPPLY1_VOLTAGE              "VS1C"
#define KEY_CPU_VRMSUPPLY2_VOLTAGE              "VS2C"

// GPU
#define KEY_GPU0_VOLTAGE                        "VC0G" // GPU 0 Core
#define KEY_GPU1_VOLTAGE                        "VC1G" // GPU 1 Core

#define KEY_NORTHBRIDGE_VOLTAGE                 "VN0C" // 
#define KEY_PCH_VOLTAGE                         "VN1C" // PCH 1.05V S0, VS1C-PP1V05 S0 SB, VV1R-1.05 S0

#define KEY_POWER_BATTERY_VOLTAGE               "VP0R" // Power/Battery (iStat)
#define KEY_DCIN_12V_S0_VOLTAGE                 "VDPR" // VD2R-Power Supply 12V S0 VD0R DC In VDPR AC/DC
#define KEY_DCIN_3V3_S5_VOLTAGE                 "VS8C" // PP3V3 S5 SB

#define KEY_MEMORY_VOLTAGE                      "VM0R" // Memory
#define KEY_POWERBATTERY_VOLTAGE                "VP0R" // Power/Battery
#define KEY_POWERSUPPLY1_VOLTAGE                "Vp0C" // Power Supply 1
#define KEY_POWERSUPPLY2_VOLTAGE                "Vp1C" // Power Supply 2
#define KEY_POWERSUPPLY3_VOLTAGE                "Vp2C" // Power Supply 3
#define KEY_POWERSUPPLY4_VOLTAGE                "Vp3C" // Power Supply 4

// Current (Amps)
#define KEY_CPU_CURRENT                         "IC0C" // CPU 0 Core
#define KEY_CPU_CURRENT_RAW                     "IC0c" // CPU 0 I-Sense
#define KEY_CPU_VCCIO_CURRENT                   "IC1C" // CPU VccIO PP1V05
#define KEY_CPU_VCCSA_CURRENT                   "IC2C" // CPU VCCSA
#define KEY_CPU_DRAM_CURRENT                    "IC5R" // VCSR-DIMM 1.5V S0
#define KEY_CPU_PLL_CURRENT                     "IC8R" // CPU 1.8V S0

#define KEY_CPU_VCORE_VTT_CURRENT               "IC0R"

// Power (Watts)
#define KEY_CPU_PACKAGE_CORE_POWER              "PCPC" // SNB
#define KEY_CPU_PACKAGE_GFX_POWER               "PCPG" // SNB
#define KEY_CPU_PACKAGE_TOTAL_POWER             "PCPT" // SNB

// FAN's
#define KEY_FAN_NUMBER                          "FNum"
#define KEY_FORMAT_FAN_ID                       "F%XID"
#define KEY_FORMAT_FAN_SPEED                    "F%XAc"

// Other
#define KEY_FORMAT_NON_APPLE_CPU_FREQUENCY		"CKC%X"
#define KEY_FORMAT_NON_APPLE_GPU_FREQUENCY      "CKG%X"

#define KEY_FORMAT_NON_APPLE_CPU_MULTIPLIER		"MU%XC"
#define KEY_NON_APPLE_CPU_PACKAGE_MULTIPLIER    "MUCP"

// Types
#define TYPE_FPE2                               "fpe2"
#define TYPE_FP2E                               "fp2e"
#define TYPE_FP4C                               "fp4c"
#define TYPE_CH8                                "ch8*"
#define TYPE_SP78                               "sp78"
#define TYPE_UI8                                "ui8"
#define TYPE_UI16                               "ui16"
#define TYPE_UI32                               "ui32"
#define TYPE_SI16                               "si16"
#define TYPE_FLAG                               "flag"

// Protocol
#define kFakeSMCDeviceService                   "FakeSMCDevice"
#define kFakeSMCDeviceValues                    "Values"
#define kFakeSMCDeviceUpdateKeyValue            "updateKeyValue"
#define kFakeSMCDevicePopulateValues            "populateValues"

#define kFakeSMCAddKeyValue                     "FakeSMC_AddKeyValue"
#define kFakeSMCAddKeyHandler                   "FakeSMC_AddKeyHandler"
#define kFakeSMCSetKeyValue                     "FakeSMC_SetKeyValue"
#define kFakeSMCGetKeyValue                     "FakeSMC_GetKeyValue"
#define kFakeSMCGetValueCallback                "FakeSMC_GetValueCallback"
#define kFakeSMCSetValueCallback                "FakeSMC_SetValueCallback"
#define kFakeSMCRemoveHandler                   "FakeSMC_RemoveHandler"

#endif
