//
//  FakeSMCDefinitions.h
//  HWSensors
//
//  Created by kozlek on 20.10.11.
//  Copyright (c) 2011 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#ifndef HWSensors_definitions_h
#define HWSensors_definitions_h

#define KEY_COUNTER                             "#KEY"

// Temperature (*C)
// CPU
#define KEY_FORMAT_CPU_DIODE_TEMPERATURE        "TC%XD" // CPU Core %X
#define KEY_FORMAT_CPU_ANALOG_TEMPERATURE       "TC%XH" // CPU Core %X
#define KEY_CPU_HEATSINK_TEMPERATURE            "Th0H"
#define KEY_FORMAT_CPU_HEATSINK_TEMPERATURE     "Th%XH"
#define KEY_CPU_PROXIMITY_TEMPERATURE           "TC0P"
#define KEY_FORMAT_CPU_PROXIMITY_TEMPERATURE    "TC%XP"
#define KEY_CPU_PACKAGE_TEMPERATURE             "TCPC"

// GPU
#define KEY_GPU_DIODE_TEMPERATURE               "TG0D"
#define KEY_FORMAT_GPU_DIODE_TEMPERATURE        "TG%XD"
#define	KEY_GPU_HEATSINK_TEMPERATURE            "TG0H"
#define	KEY_FORMAT_GPU_HEATSINK_TEMPERATURE		"TG%XH"
#define KEY_GPU_PROXIMITY_TEMPERATURE           "TG0P"
#define KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE    "TG%XP"
#define KEY_GPU_MEMORY_TEMPERATURE              "TG0M"
#define KEY_FORMAT_GPU_MEMORY_TEMPERATURE       "TG%XM"

// PECI
#define KEY_FORMAT_CPU_PECI_CORE_TEMPERATURE    "TC%Xc" // SNB
#define KEY_PECI_GFX_TEMPERATURE                "TCGc"  // SNB HD2/3000
#define KEY_PECI_SA_TEMPERATURE                 "TCSc"  // SNB 
#define KEY_PECI_PACKAGE_TEMPERATURE            "TCXc"  // SNB

// HDD
#define KEY_FORMAT_HD_BAY_TEMPERATURE           "TH%XP"
#define KEY_OPTICAL_DRIVE_TEMPERATURE           "TO0P"

// NorthBridge, MCH, MCP, PCH
#define KEY_MCH_DIODE_TEMPERATURE               "TN0C"
#define KEY_MCH_HEATSINK_TEMPERATURE            "TN0H"

#define KEY_MCP_INTERNAL_DIE_TEMPERATURE        "TN1D"
#define KEY_MCP_PROXIMITY_TEMPERATURE           "TM0P"

#define KEY_NORTHBRIDGE_PROXIMITY_TEMPERATURE   "TN0P"
#define KEY_FORMAT_NORTBRDGE_PROXY_TEMPERATURE  "TN%XP"
#define KEY_NORTHBRIDGE_TEMPERATURE             "TN0D"
#define KEY_FORMAT_NORTHBRIDGE_TEMPERATURE      "TN%XD"

#define KEY_PCH_DIE_TEMPERATURE                 "TP0D" // SNB PCH Die Digital
#define KEY_PCH_PROXIMITY_TEMPERATURE           "TP0P" // SNB

#define KEY_MAINBOARD_TEMPERATURE               "Tm0P"

// Misc
#define KEY_FORMAT_ACDC_TEMPERATURE             "Tp%XC" // PSMI Supply AC/DC Supply 1
#define KEY_AMBIENT_TEMPERATURE                 "TA0p"
#define KEY_FORMAT_AMBIENT_TEMPERATURE          "TA%Xp"
#define KEY_MLB_PROXIMITY_TEMPERATURE           "Tm0p" // MLB Proximity/DC In Proximity Airflow
#define KEY_FORMAT_AIRVENT_TEMPERATURE          "TV%XP" // Air Vent Exit
#define KEY_FORMAT_AIRPORT_TEMPERATURE          "TW%XP"
#define KEY_FORMAT_DIMM_TEMPERATURE             "TM%XS" // FB-DIMM Slot

// Custom
#define KEY_FORMAT_THERMALZONE_TEMPERATURE      "TZ%XC"

// Voltage (Volts)
// CPU
#define KEY_CPU_VOLTAGE                         "VC0C" // CPU 0
#define KEY_FORMAT_CPU_VOLTAGE                  "VC%XC" // CPU X

#define KEY_CPU_VCORE_VOLTAGE                   "VDPR" //"VC0C" // CPU 0
#define KEY_CPU_1V5_S0_VOLTAGE                  "VC5R" // CPU 1.5V S0
#define KEY_CPU_1V8_S0_VOLTAGE                  "VC8R" // CPU 1.8V S0
#define KEY_CPU_VRMSUPPLY_VOLTAGE               "VS0C" // Power Supply X
#define KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE        "VS%XC" // Power Supply X

// GPU
#define KEY_GPU_VOLTAGE                         "VC0G" // GPU 0 Core
#define KEY_FORMAT_GPU_VOLTAGE                  "VC%XG" // GPU X Core

// Others
#define KEY_NORTHBRIDGE_VOLTAGE                 "VN0R"  
#define KEY_MCH_VOLTAGE                         "VN1C"
#define KEY_POWERBATTERY_VOLTAGE                "Vb0R" // CMOS Battery

#define KEY_MAIN_12V_VOLTAGE                    "VV9S"
#define KEY_PCIE_12V_VOLTAGE                    "VeES"
#define KEY_MAIN_3V3_VOLTAGE                    "VV2S"
#define KEY_AUXILIARY_3V3V_VOLTAGE              "VV7S"
#define KEY_MAIN_5V_VOLTAGE                     "VV1S"
#define KEY_STANDBY_5V_VOLTAGE                  "VV8S"

#define KEY_MEMORY_VOLTAGE                      "VM0R" // Memory
#define KEY_POWERSUPPLY_VOLTAGE                 "Vp0C" // Power Supply X
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

// FANs
#define KEY_FAN_NUMBER                          "FNum"
#define KEY_FORMAT_FAN_ID                       "F%XID"
#define KEY_FORMAT_FAN_SPEED                    "F%XAc"

// Other
#define KEY_FAKESMC_FORMAT_CPU_FREQUENCY		"CC%XC"
#define KEY_FAKESMC_CPU_PACKAGE_FREQUENCY       "CCPC"

#define KEY_FAKESMC_GPU_FREQUENCY               "CG0C"
#define KEY_FAKESMC_FORMAT_GPU_FREQUENCY        "CG%XC"
#define KEY_FAKESMC_GPU_MEMORY_FREQUENCY        "CG0M"
#define KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY "CG%XM"
#define KEY_FAKESMC_GPU_SHADER_FREQUENCY        "CG0S"
#define KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY "CG%XS"
#define KEY_FAKESMC_GPU_ROP_FREQUENCY           "CG0R"
#define KEY_FAKESMC_FORMAT_GPU_ROP_FREQUENCY    "CG%XR"

#define KEY_FAKESMC_FORMAT_CPU_MULTIPLIER		"MlC%X"
#define KEY_FAKESMC_CPU_PACKAGE_MULTIPLIER      "MlCP"

// Types
#define TYPE_FPE2                               "fpe2"
#define TYPE_FP2E                               "fp2e"
#define TYPE_FP4C                               "fp4c"
#define TYPE_CH8                                "ch8*"
#define TYPE_SP78                               "sp78"
#define TYPE_SP4B                               "sp4b"
#define TYPE_FP5B                               "fp5b"
#define TYPE_FP88                               "fp88"
#define TYPE_UI8                                "ui8"
#define TYPE_UI16                               "ui16"
#define TYPE_UI32                               "ui32"
#define TYPE_SI16                               "si16"
#define TYPE_FLAG                               "flag"
#define TYPE_FDS                                "{fds"

#define TYPE_FPXX_SIZE                          2
#define TYPE_SPXX_SIZE                          2
#define TYPE_UI8_SIZE                           1
#define TYPE_UI16_SIZE                          2
#define TYPE_UI32_SIZE                          4
#define TYPE_SI8_SIZE                           1
#define TYPE_SI16_SIZE                          2
#define TYPE_SI32_SIZE                          4
#define TYPE_FLAG_SIZE                          1

// Services
#define kFakeSMCService                         "FakeSMC"
#define kFakeSMCDeviceService                   "FakeSMCDevice"

// I/O Registry
#define kFakeSMCDeviceValues                    "Values"
#define kFakeSMCDeviceUpdateKeyValue            "updateKeyValue"
#define kFakeSMCDevicePopulateValues            "populateValues"

// Protocol
#define kFakeSMCAddKeyValue                     "kFakeSMCAddKeyValue"
#define kFakeSMCAddKeyHandler                   "kFakeSMCAddKeyHandler"
#define kFakeSMCSetKeyValue                     "kFakeSMCSetKeyValue"
#define kFakeSMCGetKeyValue                     "kFakeSMCGetKeyValue"
#define kFakeSMCGetKeyHandler                   "kFakeSMCGetKeyHandler"
#define kFakeSMCRemoveKeyHandler                "kFakeSMCRemoveKeyHandler"
#define kFakeSMCTakeVacantGPUIndex              "kFakeSMCTakeVacantGPUIndex"
#define kFakeSMCReleaseGPUIndex                 "kFakeSMCReleaseGPUIndex"
#define kFakeSMCTakeVacantFanIndex              "kFakeSMCTakeVacantFanIndex"
#define kFakeSMCReleaseFanIndex                 "kFakeSMCReleaseFanIndex"
#define kFakeSMCGetValueCallback                "kFakeSMCGetValueCallback"
#define kFakeSMCSetValueCallback                "kFakeSMCSetValueCallback"

#define kHWSensorsDebug FALSE

#define HWSensorsDebugLog(string, args...)	do { if (kHWSensorsDebug) { IOLog ("%s: [Debug] " string "\n",getName() , ## args); } } while(0)
#define HWSensorsWarningLog(string, args...) do { IOLog ("%s: [Warning] " string "\n",getName() , ## args); } while(0)
#define HWSensorsErrorLog(string, args...) do { IOLog ("%s: [Error] " string "\n",getName() , ## args); } while(0)
#define HWSensorsFatalLog(string, args...) do { IOLog ("%s: [Fatal] " string "\n",getName() , ## args); } while(0)
#define HWSensorsInfoLog(string, args...)	do { IOLog ("%s: " string "\n",getName() , ## args); } while(0)

#define HWSensorsKeyToInt(name) *((uint32_t*)name)

#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x)	(0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

typedef enum {
    LEFT_LOWER_FRONT, CENTER_LOWER_FRONT, RIGHT_LOWER_FRONT,
    LEFT_MID_FRONT,   CENTER_MID_FRONT,   RIGHT_MID_FRONT,
    LEFT_UPPER_FRONT, CENTER_UPPER_FRONT, RIGHT_UPPER_FRONT,
    LEFT_LOWER_REAR,  CENTER_LOWER_REAR,  RIGHT_LOWER_REAR,
    LEFT_MID_REAR,    CENTER_MID_REAR,    RIGHT_MID_REAR,
    LEFT_UPPER_REAR,  CENTER_UPPER_REAR,  RIGHT_UPPER_REAR
} FanLocationType;

typedef enum {
    FAN_PWM_TACH,
    FAN_RPM,
    PUMP_PWM,
    PUMP_RPM,
    FAN_PWM_NOTACH,
    EMPTY_PLACEHOLDER,
    GPU_FAN_RPM,
    GPU_FAN_PWM_CYCLE,
} FanType;

#define DIAG_FUNCTION_STR_LEN 12

typedef struct fanTypeDescStruct {
    unsigned char   type;
    unsigned char   ui8Zone;
    unsigned char   location;
    unsigned char   rsvd;   // padding to get us to 16 bytes
    char            strFunction[DIAG_FUNCTION_STR_LEN];
} FanTypeDescStruct;

#endif
