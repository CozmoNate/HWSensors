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
    unsigned char       count;
    unsigned char       offset;
    unsigned char       shift;
} SMCKeyInfo;

const SMCKeyInfo SMCKeyInfoList[] =
{
    // Temperature
    {"TA0P", "sp78", 2, "Ambient", kSMCKeyGroupTemperature, 0, 0, 1},
    
    {"TC%XD", "sp78", 2, "CPU Die %X", kSMCKeyGroupTemperature, 8, 0, 1},
    
    {"TC%XD", "sp78", 2, "CPU %X", kSMCKeyGroupTemperature, 2, 0xA, 1},
    {"TC0H", "sp78", 2, "CPU Heatsink", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TC%XH", "sp78", 2, "CPU %X Heatsink", kSMCKeyGroupTemperature, 1, 1, 2},
    {"TCAH", "sp78", 2, "CPU Heatsink", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TC%XH", "sp78", 2, "CPU %X Heatsink", kSMCKeyGroupTemperature, 1, 0xB, 2},
    {"Th0H", "sp78", 2, "CPU Heatsink", kSMCKeyGroupTemperature, 0, 0, 1},
    {"Th%XH", "sp78", 2, "CPU %X Heatsink", kSMCKeyGroupTemperature, 1, 1, 2},
    {"TC0P", "sp78", 2, "CPU Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TC%XP", "sp78", 2, "CPU %X Proximity", kSMCKeyGroupTemperature, 1, 1, 2},
    
    {"TCXC", "sp78", 2, "PECI CPU", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TCSC", "sp78", 2, "PECI SA", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TCGC", "sp78", 2, "PECI GPU", kSMCKeyGroupTemperature, 0, 0, 1},
    
    {"TN0D", "sp78", 2, "Northbridge Die", kSMCKeyGroupTemperature, 0, 0, 1}, // ?
    {"TN%XD", "sp78", 2, "Northbridge %X Die", kSMCKeyGroupTemperature, 1, 1, 2}, // ?
    {"TN0P", "sp78", 2, "Northbridge Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TN%XP", "sp78", 2, "Northbridge %X Proximity", kSMCKeyGroupTemperature, 1, 1, 2},
    {"TN0C", "sp78", 2, "MCH Die", kSMCKeyGroupTemperature, 0, 0, 1}, // ?
    {"TN0H", "sp78", 2, "MCH Heatsink", kSMCKeyGroupTemperature, 0, 0, 1}, // ?
    {"TP0D", "sp78", 2, "PCH Die", kSMCKeyGroupTemperature, 0, 0, 1}, // ?
    {"TPCD", "sp78", 2, "PCH Die", kSMCKeyGroupTemperature, 0, 0, 1}, // ?
    {"TP0P", "sp78", 2, "PCH Proximity", kSMCKeyGroupTemperature, 0, 0, 1}, // ?
    {"Tm0P", "sp78", 2, "Mainboard", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TL0p", "sp78", 2, "LCD Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TW0P", "sp78", 2, "Airport Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"Tp0P", "sp78", 2, "Power Supply Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    
    {"TM%XP", "sp78", 2, "Memory Bank %X", kSMCKeyGroupTemperature, 8, 0, 1}, // ?
    {"TM%XS", "sp78", 2, "Memory Module %X", kSMCKeyGroupTemperature, 8, 0, 1}, // ?
    {"TMA%X", "sp78", 2, "Memory Slot %X", kSMCKeyGroupTemperature, 4, 1, 0}, // ?
    {"TMB%X", "sp78", 2, "Memory Slot %X", kSMCKeyGroupTemperature, 4, 1, 4}, // ?
    
    {"TO0P", "sp78", 2, "Optical Drive", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TH%XP", "sp78", 2, "HDD Bay %X", kSMCKeyGroupTemperature, 8, 0, 1},
    
    {"TG0D", "sp78", 2, "GPU Die", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XD", "sp78", 2, "GPU %X Die", kSMCKeyGroupTemperature, 4, 1, 2},
    {"TG0H", "sp78", 2, "GPU Heatsink", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XH", "sp78", 2, "GPU %X Heatsink", kSMCKeyGroupTemperature, 4, 1, 2},
    {"TG0P", "sp78", 2, "GPU Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XP", "sp78", 2, "GPU %X Proximity", kSMCKeyGroupTemperature, 4, 1, 2},
    {"TG0M", "sp78", 2, "GPU Memory", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XM", "sp78", 2, "GPU %X Memory", kSMCKeyGroupTemperature, 4, 1, 2},
    
    {"Te%XS", "sp78", 2, "PCIe SLot %X", kSMCKeyGroupTemperature, 4, 1, 2}, // ?
    
    {"TZ%XC", "sp78", 2, "Thermal Zone %X", kSMCKeyGroupTemperature, 0xf, 0, 1}, // ACPISensors

    // Multiplier
    
    {"MlC%X", "ui8", 1, "CPU Core %X", kSMCKeyGroupMultiplier, 8, 0, 1},
    {"MC%XC", "ui8", 1, "CPU Core %X", kSMCKeyGroupMultiplier, 8, 0, 1}, // legacy
    {"MlCP", "ui8", 1, "CPU Package Multiplier", kSMCKeyGroupMultiplier, 0, 0, 1},
    {"MPkC", "ui8", 1, "CPU Package Multiplier", kSMCKeyGroupMultiplier, 0, 0, 1}, // legacy
    
    // Frequency
    
    {"CC%XC", "ui16", 2, "CPU Core %X", kSMCKeyGroupFrequency, 8, 0, 1},
    {"CCPC", "ui16", 2, "CPU Package", kSMCKeyGroupFrequency, 0, 0, 1},
    
    {"CG0C", "ui16", 2, "GPU Core", kSMCKeyGroupFrequency, 0, 0, 1},
    {"CG%XC", "ui16", 2, "GPU %X Core", kSMCKeyGroupFrequency, 4, 1, 2},
    {"CG0M", "ui16", 2, "GPU Memory", kSMCKeyGroupFrequency, 0, 0, 1},
    {"CG%XM", "ui16", 2, "GPU %X Memory", kSMCKeyGroupFrequency, 4, 1, 2},
    {"CG0S", "ui16", 2, "GPU Shader", kSMCKeyGroupFrequency, 0, 0, 1},
    {"CG%XS", "ui16", 2, "GPU %X Shader", kSMCKeyGroupFrequency, 4, 1, 2},
    {"CG0R", "ui16", 2, "GPU ROP", kSMCKeyGroupFrequency, 0, 0, 1},
    {"CG%XR", "ui16", 2, "GPU %X ROP", kSMCKeyGroupFrequency, 4, 1, 2},
    
    // Voltage
    
    {"VC0C", "sp1e", 2, "CPU Core", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VC%XC", "sp1e", 2, "CPU %X Core", kSMCKeyGroupVoltage, 8, 1, 2},
    {"VC%XC", "sp1e", 2, "CPU %X", kSMCKeyGroupVoltage, 2, 0xA, 1},
    {"VM0R", "sp1e", 2, "Memory Modules", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VN0C", "sp1e", 2, "MCP Core", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VV2S", "sp4b", 2, "Main 3.3V", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VV1S", "sp4b", 2, "Main 5V", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VV9S", "sp4b", 2, "Main 12V", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VV7S", "sp4b", 2, "Auxiliary 3.3V", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VV8S", "sp4b", 2, "Standby 5V", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VeES", "sp4b", 2, "PCIe 12V", kSMCKeyGroupVoltage, 0, 0, 1},
    //{"VN1R", "sp4b", 2, "Power Supply 12V Rail", kSMCKeyGroupVoltage, 0}, // ?
    {"VV1R", "sp4b", 2, "Power Supply 12V", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VD0R", "sp4b", 2, "DC In S0 Rail", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VD5R", "sp4b", 2, "DC In S5 Rail", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VC0G", "sp1e", 2, "GPU Core", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VC%XG", "sp1e", 2, "GPU %X Core", kSMCKeyGroupVoltage, 4, 1, 2},
    {"VG0R", "sp4b", 2, "GPU Rail", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VG%XR", "sp4b", 2, "GPU %X Rail", kSMCKeyGroupVoltage, 4, 1, 2},
    
    {"VP0R", "sp1e", 2, "CMOS Battery", kSMCKeyGroupVoltage, 0, 0, 1}, // LPCSensors
    
    {"VS%XC", "sp4b", 2, "VRM Supply %X", kSMCKeyGroupVoltage, 0xf, 0, 1}, // ?
    {"Vp%XC", "sp4b", 2, "Power Supply %X", kSMCKeyGroupVoltage, 0xf, 0, 1}, // ?
    
    // TODO: Amperage
    
    
    
    // TODO: Power
    {"PC%XC", "fp88", 2, "CPU %X Core", kSMCKeyGroupVoltage, 2, 0, 1}, // ?
    {"PC%XC", "fp88", 2, "CPU %X Core", kSMCKeyGroupVoltage, 2, 0xA, 1}, // ?
    {"PN0C", "fp88", 2, "Northbridge", kSMCKeyGroupTemperature, 0, 0, 1}, // ?
    {"PM%XS", "fp88", 2, "DIMM Riser %X 12V", kSMCKeyGroupVoltage, 2, 0xA, 1}, // ?
    {"Pe%XS", "fp88", 2, "PCIe Slot %X +12V", kSMCKeyGroupTemperature, 4, 1, 1}, // ?
    {"Pe%XS", "fp88", 2, "PCIe Slot Boost %X +12V", kSMCKeyGroupTemperature, 2, 0xA, 1}, // ?
    {"Pp%XC", "fp88", 2, "Power Supply %X", kSMCKeyGroupVoltage, 2, 0, 1}, // ?
};

#endif
