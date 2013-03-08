//
//  SmcKeysDefinitions.h
//  HWSensors
//
//  Created by kozlek on 03.03.13.
//
//

#ifndef HWSensors_SmcKeysDefinitions_h
#define HWSensors_SmcKeysDefinitions_h

typedef enum {
    kSMCKeyGroupMisc,
    kSMCKeyGroupTemperature,
    kSMCKeyGroupMultiplier,
    kSMCKeyGroupFrequency,
    kSMCKeyGroupVoltage,
    kSMCKeyGroupAmperage,
    kSMCKeyGroupPower,
} SMCKeyGroup;

typedef struct {
    const char          *key;
    const char          *type;
    UInt8               size;
    const char          *title;
    SMCKeyGroup         group;
    unsigned char       limit;
} SMCKeyInfo;

const SMCKeyInfo SMCKeyInfoList[] =
{
    // Temperature
    {"TA0P", "sp78", 2, "Ambient", kSMCKeyGroupTemperature, 0},
    
    {"TC%XD", "sp78", 2, "CPU Die %X", kSMCKeyGroupTemperature, 0x80},
    
    {"TC%XD", "sp78", 2, "CPU %X", kSMCKeyGroupTemperature, 0x2A},
    {"TC0H", "sp78", 2, "CPU Heatsink", kSMCKeyGroupTemperature, 0},
    {"TC%XH", "sp78", 2, "CPU %X Heatsink", kSMCKeyGroupTemperature, 0x21},
    {"Th0H", "sp78", 2, "CPU Heatsink", kSMCKeyGroupTemperature, 0},
    {"Th%XH", "sp78", 2, "CPU %X Heatsink", kSMCKeyGroupTemperature, 0x21},
    {"TC0P", "sp78", 2, "CPU Proximity", kSMCKeyGroupTemperature, 0},
    {"TC%XP", "sp78", 2, "CPU %X Proximity", kSMCKeyGroupTemperature, 0x21},
    
    {"TCXC", "sp78", 2, "PECI CPU", kSMCKeyGroupTemperature, 0},
    {"TCSC", "sp78", 2, "PECI SA", kSMCKeyGroupTemperature, 0},
    {"TCGC", "sp78", 2, "PECI GPU", kSMCKeyGroupTemperature, 0},
    
    {"TN0P", "sp78", 2, "Northbridge", kSMCKeyGroupTemperature, 0},
    {"TN%XP", "sp78", 2, "Northbridge %X", kSMCKeyGroupTemperature, 0x21},
    {"TN0C", "sp78", 2, "MCH Die", kSMCKeyGroupTemperature, 0}, // ?
    {"TN0H", "sp78", 2, "MCH Heatsink", kSMCKeyGroupTemperature, 0}, // ?
    {"TN0D", "sp78", 2, "MCP Die", kSMCKeyGroupTemperature, 0}, // ?
    {"TN1D", "sp78", 2, "MCP Internal Die", kSMCKeyGroupTemperature, 0}, // ?
    {"TP0D", "sp78", 2, "PCH Die", kSMCKeyGroupTemperature, 0}, // ?
    {"TPCD", "sp78", 2, "PCH Die", kSMCKeyGroupTemperature, 0}, // ?
    {"TP0P", "sp78", 2, "PCH Proximity", kSMCKeyGroupTemperature, 0}, // ?
    {"Tm0P", "sp78", 2, "Mainboard", kSMCKeyGroupTemperature, 0},
    {"TL0p", "sp78", 2, "LCD Proximity", kSMCKeyGroupTemperature, 0},
    {"TW0P", "sp78", 2, "Airport Proximity", kSMCKeyGroupTemperature, 0},
    {"Tp0P", "sp78", 2, "Power Supply Proximity", kSMCKeyGroupTemperature, 0},
    
    {"TM%XP", "sp78", 2, "Memory Bank %X", kSMCKeyGroupTemperature, 0x80}, // ?
    {"TM%XS", "sp78", 2, "Memory Module %X", kSMCKeyGroupTemperature, 0x80}, // ?
    
    {"TO0P", "sp78", 2, "Optical Drive", kSMCKeyGroupTemperature, 0},
    {"TH%XP", "sp78", 2, "HDD Bay %X", kSMCKeyGroupTemperature, 0x80},
    
    {"TG0D", "sp78", 2, "GPU Die", kSMCKeyGroupTemperature, 0},
    {"TG%XD", "sp78", 2, "GPU %X Die", kSMCKeyGroupTemperature, 0x41},
    {"TG0H", "sp78", 2, "GPU Heatsink", kSMCKeyGroupTemperature, 0},
    {"TG%XH", "sp78", 2, "GPU %X Heatsink", kSMCKeyGroupTemperature, 0x41},
    {"TG0P", "sp78", 2, "GPU Proximity", kSMCKeyGroupTemperature, 0},
    {"TG%XP", "sp78", 2, "GPU %X Proximity", kSMCKeyGroupTemperature, 0x41},
    {"TG0M", "sp78", 2, "GPU Memory", kSMCKeyGroupTemperature, 0},
    {"TG%XM", "sp78", 2, "GPU %X Memory", kSMCKeyGroupTemperature, 0x41},
    
    {"TZ%XC", "sp78", 2, "Thermal Zone %X", kSMCKeyGroupTemperature, 0xf0}, // ACPISensors

    // Multiplier
    
    {"MlC%X", "ui8", 1, "CPU Core %X", kSMCKeyGroupMultiplier, 0x80},
    {"MC%XC", "ui8", 1, "CPU Core %X", kSMCKeyGroupMultiplier, 0x80}, // legacy
    {"MlCP", "ui8", 1, "CPU Package Multiplier", kSMCKeyGroupMultiplier, 0},
    {"MPkC", "ui8", 1, "CPU Package Multiplier", kSMCKeyGroupMultiplier, 0}, // legacy
    
    // Frequency
    
    {"CC%XC", "ui16", 2, "CPU Core %X", kSMCKeyGroupFrequency, 0x80},
    {"CCPC", "ui16", 2, "CPU Package", kSMCKeyGroupFrequency, 0},
    
    {"CG0C", "ui16", 2, "GPU Core", kSMCKeyGroupFrequency, 0},
    {"CG%XC", "ui16", 2, "GPU %X Core", kSMCKeyGroupFrequency, 0x41},
    {"CG0M", "ui16", 2, "GPU Memory", kSMCKeyGroupFrequency, 0},
    {"CG%XM", "ui16", 2, "GPU %X Memory", kSMCKeyGroupFrequency, 0x41},
    {"CG0S", "ui16", 2, "GPU Shader", kSMCKeyGroupFrequency, 0},
    {"CG%XS", "ui16", 2, "GPU %X Shader", kSMCKeyGroupFrequency, 0x41},
    {"CG0R", "ui16", 2, "GPU ROP", kSMCKeyGroupFrequency, 0},
    {"CG%XR", "ui16", 2, "GPU %X ROP", kSMCKeyGroupFrequency, 0x41},
    
    // Voltage
    
    {"VC0C", "sp1e", 2, "CPU Core", kSMCKeyGroupVoltage, 0},
    {"VC%XC", "sp1e", 2, "CPU %X Core", kSMCKeyGroupVoltage, 0x81},
    {"VC%XC", "sp1e", 2, "CPU %X", kSMCKeyGroupVoltage, 0x2A},
    {"VM0R", "sp1e", 2, "Memory Modules", kSMCKeyGroupVoltage, 0},
    {"VN0C", "sp1e", 2, "MCP Core", kSMCKeyGroupVoltage, 0},
    {"VV2S", "sp4b", 2, "Main 3.3V", kSMCKeyGroupVoltage, 0},
    {"VV1S", "sp4b", 2, "Main 5V", kSMCKeyGroupVoltage, 0},
    {"VV9S", "sp4b", 2, "Main 12V", kSMCKeyGroupVoltage, 0},
    {"VV7S", "sp4b", 2, "Auxiliary 3.3V", kSMCKeyGroupVoltage, 0},
    {"VV8S", "sp4b", 2, "Standby 5V", kSMCKeyGroupVoltage, 0},
    {"VeES", "sp4b", 2, "PCIe 12V", kSMCKeyGroupVoltage, 0},
    //{"VN1R", "sp4b", 2, "Power Supply 12V Rail", kSMCKeyGroupVoltage, 0}, // ?
    {"VV1R", "sp4b", 2, "Power Supply 12V", kSMCKeyGroupVoltage, 0},
    {"VD0R", "sp4b", 2, "DC In S0 Rail", kSMCKeyGroupVoltage, 0},
    {"VD5R", "sp4b", 2, "DC In S5 Rail", kSMCKeyGroupVoltage, 0},
    {"VC0G", "sp1e", 2, "GPU Core", kSMCKeyGroupVoltage, 0},
    {"VC%XG", "sp1e", 2, "GPU %X Core", kSMCKeyGroupVoltage, 0x41},
    {"VG0R", "sp4b", 2, "GPU Rail", kSMCKeyGroupVoltage, 0},
    {"VG%XR", "sp4b", 2, "GPU %X Rail", kSMCKeyGroupVoltage, 0x41},
    
    {"VP0R", "sp1e", 2, "CMOS Battery", kSMCKeyGroupVoltage, 0}, // LPCSensors
    
    {"VS%XC", "sp4b", 2, "VRM Supply %X", kSMCKeyGroupVoltage, 0xf0}, // ?
    {"Vp%XC", "sp4b", 2, "Power Supply %X", kSMCKeyGroupVoltage, 0xf0}, // ?
    
    // TODO: Amperage
    
    
    
    // TODO: Power
    
    
};

#endif
