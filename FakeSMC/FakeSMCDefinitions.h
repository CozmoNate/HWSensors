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
#define KEY_CPU_VOLTAGE                         "VC0C" // CPU 0
#define KEY_FORMAT_CPU_VOLTAGE                  "VC%XC" // CPU X

#define KEY_CPU_VCORE_VOLTAGE                   "VDPR"//"VC0C" // CPU 0
#define KEY_CPU_1V8_S0_VOLTAGE                  "VC8R" // CPU 1.8V S0
#define KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE        "VS%XC" // Power Supply X

// GPU
#define KEY_GPU_VOLTAGE                         "VC0G" // GPU 0 Core
#define KEY_FORMAT_GPU_VOLTAGE                  "VC%XG" // GPU X Core

#define KEY_NORTHBRIDGE_VOLTAGE                 "VN0R"  
#define KEY_MCH_VOLTAGE                         "VN1C"
#define KEY_POWERBATTERY_VOLTAGE                "VP0R" // Power/Battery

#define KEY_MAIN_12V_VOLTAGE                    "VV9S"
#define KEY_PCIE_12V_VOLTAGE                    "VeES"
#define KEY_MAIN_3V3_VOLTAGE                    "VV2S"
#define KEY_AUXILIARY_3V3V_VOLTAGE              "VV7S"
#define KEY_MAIN_5V_VOLTAGE                     "VV1S"
#define KEY_STANDBY_5V_VOLTAGE                  "VV8S"

#define KEY_MEMORY_VOLTAGE                      "VM0R" // Memory
#define KEY_FORMAT_POWERSUPPLY_VOLTAGE          "Vp%XC" // Power Supply X

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
#define KEY_FORMAT_FAKESMC_CPU_FREQUENCY		"ClC%X"
#define KEY_FORMAT_FAKESMC_GPU_FREQUENCY        "ClG%X"
#define KEY_FAKESMC_CPU_PACKAGE_FREQUENCY       "ClCP"

#define KEY_FORMAT_FAKESMC_CPU_MULTIPLIER		"MlC%X"
#define KEY_FAKESMC_CPU_PACKAGE_MULTIPLIER      "MlCP"

// Types
#define TYPE_FPE2                               "fpe2"
#define TYPE_FP2E                               "fp2e"
#define TYPE_FP4C                               "fp4c"
#define TYPE_CH8                                "ch8*"
#define TYPE_SP78                               "sp78"
#define TYPE_FP88                               "fp88"
#define TYPE_UI8                                "ui8"
#define TYPE_UI16                               "ui16"
#define TYPE_UI32                               "ui32"
#define TYPE_SI16                               "si16"
#define TYPE_FLAG                               "flag"

#define TYPE_FPXX_SIZE                          2
#define TYPE_SPXX_SIZE                          2
#define TYPE_UI8_SIZE                           1
#define TYPE_UI16_SIZE                          2
#define TYPE_UI32_SIZE                          4
#define TYPE_SI8_SIZE                           1
#define TYPE_SI16_SIZE                          2
#define TYPE_SI32_SIZE                          4

// Protocol
#define kFakeSMCDeviceService                   "FakeSMCDevice"
#define kFakeSMCDeviceValues                    "Values"
#define kFakeSMCDeviceUpdateKeyValue            "updateKeyValue"
#define kFakeSMCDevicePopulateValues            "populateValues"

#define kFakeSMCAddKeyValue                     "FakeSMC_AddKeyValue"
#define kFakeSMCAddKeyHandler                   "FakeSMC_AddKeyHandler"
#define kFakeSMCSetKeyValue                     "FakeSMC_SetKeyValue"
#define kFakeSMCGetKeyValue                     "FakeSMC_GetKeyValue"
#define kFakeSMCGetKeyHandler                   "FakeSMC_GetKeyHandler"
#define kFakeSMCGetValueCallback                "FakeSMC_GetValueCallback"
#define kFakeSMCSetValueCallback                "FakeSMC_SetValueCallback"
#define kFakeSMCRemoveHandler                   "FakeSMC_RemoveHandler"

#endif
