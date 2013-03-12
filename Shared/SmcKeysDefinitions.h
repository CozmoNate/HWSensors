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
    kSMCKeyGroupCurrent,
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
    {"TM%XP", "sp78", 2, "Memory Bank %X", kSMCKeyGroupTemperature, 8, 0, 1}, // ?
    {"TM%XS", "sp78", 2, "Memory Module %X", kSMCKeyGroupTemperature, 8, 0, 1}, // ?
    {"TMA%X", "sp78", 2, "Memory Slot %X", kSMCKeyGroupTemperature, 4, 1, 0}, // ?
    {"TMB%X", "sp78", 2, "Memory Slot %X", kSMCKeyGroupTemperature, 4, 1, 4}, // ?
    {"TL0p", "sp78", 2, "LCD Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TW0P", "sp78", 2, "Airport Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TO0P", "sp78", 2, "Optical Drive", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TH%XP", "sp78", 2, "HDD Bay %X", kSMCKeyGroupTemperature, 8, 0, 1},
    {"Tm0P", "sp78", 2, "Mainboard Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"Tp1P", "sp78", 2, "Powerboard Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"Tp2H", "sp78", 2, "Power Heatsink 1", kSMCKeyGroupTemperature, 0, 0, 1},
    {"Tp3H", "sp78", 2, "Power Heatsink 2", kSMCKeyGroupTemperature, 0, 0, 1},
    
    {"TG0D", "sp78", 2, "GPU Die", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XD", "sp78", 2, "GPU %X Die", kSMCKeyGroupTemperature, 3, 1, 2},
    {"TG0H", "sp78", 2, "GPU Heatsink", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XH", "sp78", 2, "GPU %X Heatsink", kSMCKeyGroupTemperature, 3, 1, 2},
    {"TG0P", "sp78", 2, "GPU Proximity", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XP", "sp78", 2, "GPU %X Proximity", kSMCKeyGroupTemperature, 3, 1, 2},
    {"TG0M", "sp78", 2, "GPU Memory", kSMCKeyGroupTemperature, 0, 0, 1},
    {"TG%XM", "sp78", 2, "GPU %X Memory", kSMCKeyGroupTemperature, 3, 1, 2},
    
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
    {"VD0R", "sp4b", 2, "Mainboard S0 Rail", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VD5R", "sp4b", 2, "Mainboard S5 Rail", kSMCKeyGroupVoltage, 0, 0, 1},
    {"Vp0C", "sp4b", 2, "12V Vcc", kSMCKeyGroupVoltage, 0, 0, 1},
    {"Vp1C", "sp4b", 2, "5V Vcc", kSMCKeyGroupVoltage, 0, 0, 1},
    {"Vp2C", "sp4b", 2, "5V VSB", kSMCKeyGroupVoltage, 0, 0, 1},
    {"Vp3C", "sp4b", 2, "3.3V Vcc", kSMCKeyGroupVoltage, 0, 0, 1},
    {"Vp4C", "sp4b", 2, "3.3V VSB", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VC0G", "sp1e", 2, "GPU Core", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VC%XG", "sp1e", 2, "GPU %X Core", kSMCKeyGroupVoltage, 3, 1, 2},
    {"VG0R", "sp4b", 2, "GPU Rail", kSMCKeyGroupVoltage, 0, 0, 1},
    {"VG%XR", "sp4b", 2, "GPU %X Rail", kSMCKeyGroupVoltage, 3, 1, 2},
    
    {"VP0R", "sp1e", 2, "CMOS Battery", kSMCKeyGroupVoltage, 0, 0, 1}, // LPCSensors
    {"VBAT", "sp4b", 2, "Battery", kSMCKeyGroupVoltage, 0, 0, 1},
    
    {"VS%XC", "sp4b", 2, "VRM Supply %X", kSMCKeyGroupVoltage, 0xf, 0, 1}, // ?
    {"Vp%XC", "sp4b", 2, "Power Supply %X", kSMCKeyGroupVoltage, 0xf, 0, 1}, // ?
    
    // Current
    {"IC0C", "sp78", 2, "CPU Core", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"IC1C", "sp78", 2, "CPU VccIO", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"IC2C", "sp78", 2, "CPU VCCSA", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"IC5R", "sp78", 2, "CPU DRAM", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"IC8R", "sp78", 2, "CPU PLL", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"IM0R", "sp5a", 2, "Memory Modules", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"IW0E", "sp5a", 2, "Airport Rail", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"ID0R", "sp5a", 2, "Mainboard S0 Rail", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"ID5R", "sp4b", 2, "Mainboard S5 Rail", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    {"IC0G", "sp69", 2, "GPU Core", kSMCKeyGroupCurrent, 0, 0, 1}, // ?
    
    
    // Power
    {"PC0C", "sp96", 2, "CPU Core", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PC%XC", "sp96", 2, "CPU %X Core", kSMCKeyGroupPower, 1, 1, 2}, // ?
    {"PCAC", "sp96", 2, "CPU Core", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PC%XC", "sp96", 2, "CPU %X Core", kSMCKeyGroupPower, 1, 0xB, 2}, // ?
    {"PCPC", "sp96", 2, "CPU Package Core", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PCPG", "sp96", 2, "CPU Package GFX", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PCPT", "sp96", 2, "CPU Package Total", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PC1R", "sp96", 2, "CPU Rail", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PC5R", "sp96", 2, "CPU 1.5V S0 Rail", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PN0C", "sp96", 2, "Northbridge", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PM0R", "sp96", 2, "Memory Rail", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PM%XS", "sp96", 2, "DIMM Riser %X 12V", kSMCKeyGroupPower, 2, 0xA, 1}, // ?
    {"PC0G", "sp96", 2, "GPU Core", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PG0R", "sp96", 2, "GPU Rail", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"Pe%XS", "sp96", 2, "PCIe Slot %X +12V", kSMCKeyGroupPower, 4, 1, 1}, // ?
    {"Pe%XS", "sp96", 2, "PCIe Slot Boost %X +12V", kSMCKeyGroupPower, 4, 0xA, 1}, // ?
    {"PD0R", "sp96", 2, "Mainboard S0 Rail", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PD5R", "sp96", 2, "Mainboard S5 Rail", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"Pp%XC", "sp96", 2, "Power Supply %X", kSMCKeyGroupPower, 2, 0, 1}, // ?
    {"PDTR", "sp96", 2, "System Total", kSMCKeyGroupPower, 0, 0, 1}, // ?
    {"PZ%XG", "sp96", 2, "Zone%X Average", kSMCKeyGroupPower, 4, 1, 1}, // ?
};

#endif
