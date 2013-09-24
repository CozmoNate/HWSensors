//
//  HWMonitorProfiles.m
//  HWMonitor
//
//  Created by kozlek on 30.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import "HWMonitorProfiles.h"

@implementation HWMonitorProfiles

// :[offset][count][shift]
+(NSDictionary *)profiles
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
           
            // Default
            [NSArray arrayWithObjects:
             // Temperatures
             [NSArray arrayWithObjects:@"TA0p",       @"Ambient", nil],
             [NSArray arrayWithObjects:@"TA0P",       @"Ambient", nil],
             [NSArray arrayWithObjects:@"TC:081D",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"TC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"TC:A41D",    @"CPU %X Package", nil],
             [NSArray arrayWithObjects:@"TC:A41C",    @"CPU %X Core", nil],
             [NSArray arrayWithObjects:@"TC:A41G",    @"CPU %X Graphics", nil],
             [NSArray arrayWithObjects:@"TC:A41H",    @"CPU %X Heatsink", nil],
             [NSArray arrayWithObjects:@"TC0H",       @"CPU Heatsink", nil],
             [NSArray arrayWithObjects:@"Th0H",       @"CPU Heatsink", nil],
             [NSArray arrayWithObjects:@"TC0P",       @"CPU Proximity", nil],
             [NSArray arrayWithObjects:@"TCXC",       @"PECI CPU", nil],
             [NSArray arrayWithObjects:@"TCXc",       @"PECI CPU", nil],
             [NSArray arrayWithObjects:@"TCSC",       @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCSc",       @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCSA",       @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCGC",       @"PECI GPU", nil],
             [NSArray arrayWithObjects:@"TCGc",       @"PECI GPU", nil],
             [NSArray arrayWithObjects:@"TN0D",       @"Northbridge Die", nil],
             [NSArray arrayWithObjects:@"TN0P",       @"Northbridge Proximity", nil],
             [NSArray arrayWithObjects:@"TN0C",       @"MCH Die", nil],
             [NSArray arrayWithObjects:@"TN0H",       @"MCH Heatsink", nil],
             [NSArray arrayWithObjects:@"TP0D",       @"PCH Die", nil],
             [NSArray arrayWithObjects:@"TPCD",       @"PCH Die", nil],
             [NSArray arrayWithObjects:@"TP0P",       @"PCH Proximity", nil],
             [NSArray arrayWithObjects:@"TM:091S",    @"Memory Module %X", nil],
             [NSArray arrayWithObjects:@"TM:091P",    @"Memory Module %X, Proximity", nil],
             [NSArray arrayWithObjects:@"TMA:051",    @"Memory Bank A, Module %X", nil],
             [NSArray arrayWithObjects:@"TMB:051",    @"Memory Bank B, Module %X", nil],
             [NSArray arrayWithObjects:@"TL0P",       @"LCD Proximity", nil],
             [NSArray arrayWithObjects:@"TW0P",       @"Airport Proximity", nil],
             [NSArray arrayWithObjects:@"TO0P",       @"Optical Drive", nil],
             [NSArray arrayWithObjects:@"TH:081P",    @"HDD Bay %X", nil],
             [NSArray arrayWithObjects:@"TB:041P",    @"Battery %X", nil],
             [NSArray arrayWithObjects:@"Tm0P",       @"Mainboard Proximity", nil],
             [NSArray arrayWithObjects:@"Tp1P",       @"Powerboard Proximity", nil],
             [NSArray arrayWithObjects:@"Tp:221H",    @"Power Heatsink %X", nil],
             [NSArray arrayWithObjects:@"Te:081S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"TG0D",       @"GPU Die", nil],
             [NSArray arrayWithObjects:@"TG0H",       @"GPU Heatsink", nil],
             [NSArray arrayWithObjects:@"TG0p",       @"GPU Proximity", nil],
             [NSArray arrayWithObjects:@"TG:041M",    @"GPU Memory", nil],
             [NSArray arrayWithObjects:@"TG:132D",    @"GPU %X Die", nil],
             [NSArray arrayWithObjects:@"TG:132H",    @"GPU %X Heatsink", nil],
             [NSArray arrayWithObjects:@"TG:132p",    @"GPU %X Proximity", nil],
             [NSArray arrayWithObjects:@"TG:132M",    @"GPU %X Memory", nil],
             [NSArray arrayWithObjects:@"TZ:0F1C",    @"Thermal Zone %X", nil],
             
             // Multipliers
             [NSArray arrayWithObjects:@"MlC:081",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"MC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"MlCP",       @"CPU Package Multiplier", nil],
             [NSArray arrayWithObjects:@"MPkC",       @"CPU Package Multiplier", nil],
             
             // Clocks
             [NSArray arrayWithObjects:@"CC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"CCPC",       @"CPU Package", nil],
             [NSArray arrayWithObjects:@"CG0C",       @"GPU Core", nil],
             [NSArray arrayWithObjects:@"CG0M",       @"GPU Memory", nil],
             [NSArray arrayWithObjects:@"CG0S",       @"GPU Shaders", nil],
             [NSArray arrayWithObjects:@"CG0R",       @"GPU ROPs", nil],
             [NSArray arrayWithObjects:@"CG:132C",    @"GPU %X Core", nil],
             [NSArray arrayWithObjects:@"CG:132M",    @"GPU %X Memory", nil],
             [NSArray arrayWithObjects:@"CG:132S",    @"GPU %X Shaders", nil],
             [NSArray arrayWithObjects:@"CG:132R",    @"GPU %X ROPs", nil],
             
             // Voltages
             [NSArray arrayWithObjects:@"VC0C",       @"CPU Core", nil],
             [NSArray arrayWithObjects:@"VC:A4AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"VV1R",       @"CPU VTT", nil],
             [NSArray arrayWithObjects:@"VN1R",       @"PCH Core", nil],
             [NSArray arrayWithObjects:@"VM0R",       @"Memory Modules", nil],
             [NSArray arrayWithObjects:@"VN0C",       @"MCH Core", nil],
             [NSArray arrayWithObjects:@"VV2S",       @"Main 3.3V", nil],
             [NSArray arrayWithObjects:@"VV1S",       @"Main 5V", nil],
             [NSArray arrayWithObjects:@"VV9S",       @"Main 12V", nil],
             [NSArray arrayWithObjects:@"VV7S",       @"Auxiliary 3.3V", nil],
             [NSArray arrayWithObjects:@"VV8S",       @"Standby 5V", nil],
             [NSArray arrayWithObjects:@"VeES",       @"PCIe 12V", nil],
             [NSArray arrayWithObjects:@"VP0R",       @"+12V Rail", nil],
             [NSArray arrayWithObjects:@"Vp0C",       @"12V Vcc", nil],
             [NSArray arrayWithObjects:@"Vp:172C",    @"Power Supply %X", nil],
             [NSArray arrayWithObjects:@"VD0R",       @"Mainboard S0 Rail", nil],
             [NSArray arrayWithObjects:@"VD5R",       @"Mainboard S5 Rail", nil],
             [NSArray arrayWithObjects:@"Vb0R",       @"CMOS Battery", nil],
             [NSArray arrayWithObjects:@"VBAT",       @"Battery", nil],
             [NSArray arrayWithObjects:@"VS0C",       @"CPU VRM", nil],
             [NSArray arrayWithObjects:@"VS:1E2C",    @"CPU VRM %X", nil],
             [NSArray arrayWithObjects:@"VC0G",       @"GPU Core", nil],
             [NSArray arrayWithObjects:@"VC:132G",    @"GPU %X Core", nil],
             [NSArray arrayWithObjects:@"VG0C",       @"GPU Core", nil],
             [NSArray arrayWithObjects:@"VG:132C",    @"GPU %X Core", nil],
             [NSArray arrayWithObjects:@"VC0R",       @"GPU Rail", nil],
             [NSArray arrayWithObjects:@"VC:132R",    @"GPU %X Rail", nil],
             
             // Currents
             [NSArray arrayWithObjects:@"IC0C",       @"CPU Core", nil],
             [NSArray arrayWithObjects:@"IC1C",       @"CPU VccIO", nil],
             [NSArray arrayWithObjects:@"IC2C",       @"CPU VccSA", nil],
             [NSArray arrayWithObjects:@"IC5R",       @"CPU DRAM", nil],
             [NSArray arrayWithObjects:@"IC8R",       @"CPU PLL", nil],
             [NSArray arrayWithObjects:@"IC:A4AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"IC0G",       @"CPU GFX", nil],
             [NSArray arrayWithObjects:@"IM:A4AS",    @"Memory Bank %X", nil],
             [NSArray arrayWithObjects:@"IN0C",       @"MCH Core", nil],
             [NSArray arrayWithObjects:@"IM0R",       @"Memory Rail", nil],
             [NSArray arrayWithObjects:@"IW0E",       @"Airport Rail", nil],
             [NSArray arrayWithObjects:@"IB0R",       @"Battery Rail", nil],
             [NSArray arrayWithObjects:@"Ie:081S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"IM:A4AS",    @"PCIe Booster %X", nil],
             [NSArray arrayWithObjects:@"IG0C",       @"GPU", nil],
             [NSArray arrayWithObjects:@"IG:132C",    @"GPU %X", nil],
             [NSArray arrayWithObjects:@"ID0R",       @"Mainboard S0 Rail", nil],
             [NSArray arrayWithObjects:@"ID5R",       @"Mainboard S5 Rail", nil],
             
             // Powers
             [NSArray arrayWithObjects:@"PC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"PC:A8AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"PC0G",       @"CPU GFX", nil],
             [NSArray arrayWithObjects:@"PCPC",       @"CPU Package Cores", nil],
             [NSArray arrayWithObjects:@"PCPG",       @"CPU Package Graphics", nil],
             [NSArray arrayWithObjects:@"PCPT",       @"CPU Package Total", nil],
             [NSArray arrayWithObjects:@"PCPD",       @"CPU Package DRAM", nil],
             [NSArray arrayWithObjects:@"PC1R",       @"CPU Rail", nil],
             [NSArray arrayWithObjects:@"PC5R",       @"CPU 1.5V S0 Rail", nil],
             [NSArray arrayWithObjects:@"PM0R",       @"Memory Rail", nil],
             [NSArray arrayWithObjects:@"PM:A4AS",    @"Memory Bank %X", nil],
             [NSArray arrayWithObjects:@"Pe:041S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"Pe:A4AS",    @"PCIe Booster %X", nil],
             [NSArray arrayWithObjects:@"PG0C",       @"GPU", nil],
             [NSArray arrayWithObjects:@"PG:132C",    @"GPU %X", nil],
             [NSArray arrayWithObjects:@"PG0R",       @"GPU Rail", nil],
             [NSArray arrayWithObjects:@"PG:132R",    @"GPU %X Rail", nil],
             [NSArray arrayWithObjects:@"PD0R",       @"Mainboard S0 Rail", nil],
             [NSArray arrayWithObjects:@"PD5R",       @"Mainboard S5 Rail", nil],
             [NSArray arrayWithObjects:@"Pp0C",       @"Power Supply 12V", nil],
             [NSArray arrayWithObjects:@"PDTR",       @"System Total", nil],
             [NSArray arrayWithObjects:@"PZ:041G",    @"Zone %X Average", nil],
             
             nil], @"Default",
            
            // Hackintosh
            [NSArray arrayWithObjects:
             // Temperatures
             [NSArray arrayWithObjects:@"TA0p",       @"Ambient", nil],
             [NSArray arrayWithObjects:@"TA0P",       @"Ambient", nil],
             [NSArray arrayWithObjects:@"TC:081D",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"TC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"TCAD",       @"CPU Package", nil],
             [NSArray arrayWithObjects:@"TC:B32D",    @"CPU %X Package", nil],
             [NSArray arrayWithObjects:@"TC:A41C",    @"CPU %X Core", nil],
             [NSArray arrayWithObjects:@"TC:A41G",    @"CPU %X Graphics", nil],
             [NSArray arrayWithObjects:@"TC:A41H",    @"CPU %X Heatsink", nil],
             [NSArray arrayWithObjects:@"TC0H",       @"CPU Heatsink", nil],
             [NSArray arrayWithObjects:@"Th0H",       @"CPU Heatsink", nil],
             [NSArray arrayWithObjects:@"TC0P",       @"CPU Proximity", nil],
             [NSArray arrayWithObjects:@"TCXC",       @"PECI CPU", nil],
             [NSArray arrayWithObjects:@"TCXc",       @"PECI CPU", nil],
             [NSArray arrayWithObjects:@"TCSC",       @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCSc",       @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCSA",       @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCGC",       @"PECI GPU", nil],
             [NSArray arrayWithObjects:@"TCGc",       @"PECI GPU", nil],
             [NSArray arrayWithObjects:@"TN0D",       @"Northbridge Die", nil],
             [NSArray arrayWithObjects:@"TN0P",       @"Northbridge Proximity", nil],
             [NSArray arrayWithObjects:@"TN0C",       @"MCH Die", nil],
             [NSArray arrayWithObjects:@"TN0H",       @"MCH Heatsink", nil],
             [NSArray arrayWithObjects:@"TP0D",       @"PCH Die", nil],
             [NSArray arrayWithObjects:@"TPCD",       @"PCH Die", nil],
             [NSArray arrayWithObjects:@"TP0P",       @"PCH Proximity", nil],
             [NSArray arrayWithObjects:@"TM:091S",    @"Memory Module %X", nil],
             [NSArray arrayWithObjects:@"TM:091P",    @"Memory Module %X, Proximity", nil],
             [NSArray arrayWithObjects:@"TMA:051",    @"Memory Bank A, Module %X", nil],
             [NSArray arrayWithObjects:@"TMB:051",    @"Memory Bank B, Module %X", nil],
             [NSArray arrayWithObjects:@"TL0P",       @"LCD Proximity", nil],
             [NSArray arrayWithObjects:@"TW0P",       @"Airport Proximity", nil],
             [NSArray arrayWithObjects:@"TO0P",       @"Optical Drive", nil],
             [NSArray arrayWithObjects:@"TH:081P",    @"HDD Bay %X", nil],
             [NSArray arrayWithObjects:@"TB:041P",    @"Battery %X", nil],
             [NSArray arrayWithObjects:@"Tm0P",       @"Mainboard Proximity", nil],
             [NSArray arrayWithObjects:@"Tp1P",       @"Powerboard Proximity", nil],
             [NSArray arrayWithObjects:@"Tp:221H",    @"Power Heatsink %X", nil],
             [NSArray arrayWithObjects:@"Te:081S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"TG0D",       @"GPU Die", nil],
             [NSArray arrayWithObjects:@"TG0H",       @"GPU Heatsink", nil],
             [NSArray arrayWithObjects:@"TG0p",       @"GPU Proximity", nil],
             [NSArray arrayWithObjects:@"TG:041M",    @"GPU Memory", nil],
             [NSArray arrayWithObjects:@"TG:132D",    @"GPU %X Die", nil],
             [NSArray arrayWithObjects:@"TG:132H",    @"GPU %X Heatsink", nil],
             [NSArray arrayWithObjects:@"TG:132p",    @"GPU %X Proximity", nil],
             [NSArray arrayWithObjects:@"TG:132M",    @"GPU %X Memory", nil],
             [NSArray arrayWithObjects:@"TZ:0F1C",    @"Thermal Zone %X", nil],
             
             // Multipliers
             [NSArray arrayWithObjects:@"MlC:081",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"MC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"MlCP",       @"CPU Package Multiplier", nil],
             [NSArray arrayWithObjects:@"MPkC",       @"CPU Package Multiplier", nil],
             
             // Clocks
             [NSArray arrayWithObjects:@"CC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"CCPC",       @"CPU Package", nil],
             [NSArray arrayWithObjects:@"CG0C",       @"GPU Core", nil],
             [NSArray arrayWithObjects:@"CG0M",       @"GPU Memory", nil],
             [NSArray arrayWithObjects:@"CG0S",       @"GPU Shaders", nil],
             [NSArray arrayWithObjects:@"CG0R",       @"GPU ROPs", nil],
             [NSArray arrayWithObjects:@"CG:132C",    @"GPU %X Core", nil],
             [NSArray arrayWithObjects:@"CG:132M",    @"GPU %X Memory", nil],
             [NSArray arrayWithObjects:@"CG:132S",    @"GPU %X Shaders", nil],
             [NSArray arrayWithObjects:@"CG:132R",    @"GPU %X ROPs", nil],
             
             // Voltages
             [NSArray arrayWithObjects:@"VC0C",       @"CPU Core", nil],
             [NSArray arrayWithObjects:@"VC:A4AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"VV1R",       @"CPU VTT", nil],
             [NSArray arrayWithObjects:@"VN1R",       @"PCH Core", nil],
             [NSArray arrayWithObjects:@"VM0R",       @"Memory Modules", nil],
             [NSArray arrayWithObjects:@"VN0C",       @"MCH Core", nil],
             [NSArray arrayWithObjects:@"VV2S",       @"Main 3.3V", nil],
             [NSArray arrayWithObjects:@"VV1S",       @"Main 5V", nil],
             [NSArray arrayWithObjects:@"VV9S",       @"Main 12V", nil],
             [NSArray arrayWithObjects:@"VV7S",       @"Auxiliary 3.3V", nil],
             [NSArray arrayWithObjects:@"VV8S",       @"Standby 5V", nil],
             [NSArray arrayWithObjects:@"VeES",       @"PCIe 12V", nil],
             [NSArray arrayWithObjects:@"VP0R",       @"+12V Rail", nil],
             [NSArray arrayWithObjects:@"Vp0C",       @"12V Vcc", nil],
             [NSArray arrayWithObjects:@"Vp:172C",    @"Power Supply %X", nil],
             [NSArray arrayWithObjects:@"VD0R",       @"Mainboard S0 Rail", nil],
             [NSArray arrayWithObjects:@"VD5R",       @"Mainboard S5 Rail", nil],
             [NSArray arrayWithObjects:@"Vb0R",       @"CMOS Battery", nil],
             [NSArray arrayWithObjects:@"VBAT",       @"Battery", nil],
             [NSArray arrayWithObjects:@"VS0C",       @"CPU VRM", nil],
             [NSArray arrayWithObjects:@"VS:1E2C",    @"CPU VRM %X", nil],
             [NSArray arrayWithObjects:@"VC0G",       @"GPU Core", nil],
             [NSArray arrayWithObjects:@"VC:132G",    @"GPU %X Core", nil],
             [NSArray arrayWithObjects:@"VG0C",       @"GPU Core", nil],
             [NSArray arrayWithObjects:@"VG:132C",    @"GPU %X Core", nil],
             [NSArray arrayWithObjects:@"VC0R",       @"GPU Rail", nil],
             [NSArray arrayWithObjects:@"VC:132R",    @"GPU %X Rail", nil],
             
             // Currents
             [NSArray arrayWithObjects:@"IC0C",       @"CPU Core", nil],
             [NSArray arrayWithObjects:@"IC1C",       @"CPU VccIO", nil],
             [NSArray arrayWithObjects:@"IC2C",       @"CPU VccSA", nil],
             [NSArray arrayWithObjects:@"IC5R",       @"CPU DRAM", nil],
             [NSArray arrayWithObjects:@"IC8R",       @"CPU PLL", nil],
             [NSArray arrayWithObjects:@"IC:A4AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"IC0G",       @"CPU GFX", nil],
             [NSArray arrayWithObjects:@"IM:A4AS",    @"Memory Bank %X", nil],
             [NSArray arrayWithObjects:@"IN0C",       @"MCH Core", nil],
             [NSArray arrayWithObjects:@"IM0R",       @"Memory Rail", nil],
             [NSArray arrayWithObjects:@"IW0E",       @"Airport Rail", nil],
             [NSArray arrayWithObjects:@"IB0R",       @"Battery Rail", nil],
             [NSArray arrayWithObjects:@"Ie:081S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"IM:A4AS",    @"PCIe Booster %X", nil],
             [NSArray arrayWithObjects:@"IG0C",       @"GPU", nil],
             [NSArray arrayWithObjects:@"IG:132C",    @"GPU %X", nil],
             [NSArray arrayWithObjects:@"ID0R",       @"Mainboard S0 Rail", nil],
             [NSArray arrayWithObjects:@"ID5R",       @"Mainboard S5 Rail", nil],
             
             // Powers
             [NSArray arrayWithObjects:@"PC:081C",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"PC:A8AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"PC0G",       @"CPU GFX", nil],
             [NSArray arrayWithObjects:@"PCPC",       @"CPU Package Cores", nil],
             [NSArray arrayWithObjects:@"PCPG",       @"CPU Package Graphics", nil],
             [NSArray arrayWithObjects:@"PCPT",       @"CPU Package Total", nil],
             [NSArray arrayWithObjects:@"PCPD",       @"CPU Package DRAM", nil],
             [NSArray arrayWithObjects:@"PC1R",       @"CPU Rail", nil],
             [NSArray arrayWithObjects:@"PC5R",       @"CPU 1.5V S0 Rail", nil],
             [NSArray arrayWithObjects:@"PM0R",       @"Memory Rail", nil],
             [NSArray arrayWithObjects:@"PM:A4AS",    @"Memory Bank %X", nil],
             [NSArray arrayWithObjects:@"Pe:041S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"Pe:A4AS",    @"PCIe Booster %X", nil],
             [NSArray arrayWithObjects:@"PG0C",       @"GPU", nil],
             [NSArray arrayWithObjects:@"PG:132C",    @"GPU %X", nil],
             [NSArray arrayWithObjects:@"PG0R",       @"GPU Rail", nil],
             [NSArray arrayWithObjects:@"PG:132R",    @"GPU %X Rail", nil],
             [NSArray arrayWithObjects:@"PD0R",       @"Mainboard S0 Rail", nil],
             [NSArray arrayWithObjects:@"PD5R",       @"Mainboard S5 Rail", nil],
             [NSArray arrayWithObjects:@"Pp0C",       @"Power Supply 12V", nil],
             [NSArray arrayWithObjects:@"PDTR",       @"System Total", nil],
             [NSArray arrayWithObjects:@"PZ:041G",    @"Zone %X Average", nil],
             
             nil], @"Hackintosh",
            
            // MacBookPro8,1
            [NSArray arrayWithObjects:
             // Temperatures
             [NSArray arrayWithObjects:@"TC0:C41",    @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"TC0P",       @"CPU Proximity", nil],
             [NSArray arrayWithObjects:@"TPCD",       @"PCH Die", nil],
             [NSArray arrayWithObjects:@"TP0P",       @"PCH Proximity", nil],
             [NSArray arrayWithObjects:@"TCSA",       @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCGC",       @"PECI GPU", nil],
             [NSArray arrayWithObjects:@"TM0S",       @"Memory Modules", nil],
             [NSArray arrayWithObjects:@"TM0P",       @"Memory Modules Proximity", nil],
             [NSArray arrayWithObjects:@"Th1H",       @"Heatpipe", nil],
             [NSArray arrayWithObjects:@"Ts0P",       @"Heatpipe Incoming Air", nil],
             [NSArray arrayWithObjects:@"Ts0S",       @"Heatpipe Outgoing Air", nil],
             [NSArray arrayWithObjects:@"TB:031T",    @"Battery Unit %X", nil],
             
             // Voltages
             [NSArray arrayWithObjects:@"VC0C",       @"CPU Core", nil],
             //[NSArray arrayWithObjects:@"VN0R",       @" ", nil], 0.5V ??
             [NSArray arrayWithObjects:@"VP0R",       @"Power Supply 12V", nil],
             [NSArray arrayWithObjects:@"VD0R",       @"Mainboard S0 Rail", nil], // 17V ??
             
             // Currents
             [NSArray arrayWithObjects:@"IC0R",       @"CPU Rail", nil],
             //[NSArray arrayWithObjects:@"IC1C",       @" ? ", nil],
             //[NSArray arrayWithObjects:@"IN0C",       @" ? ", nil],
             [NSArray arrayWithObjects:@"IB0R",       @"Battery Rail", nil],
             [NSArray arrayWithObjects:@"ID0R",       @"Mainboard S0 Rail", nil],
             
             // Powers
             [NSArray arrayWithObjects:@"PC0C",    @"CPU Core", nil],
             [NSArray arrayWithObjects:@"PCPC",    @"CPU Package Cores", nil],
             [NSArray arrayWithObjects:@"PCPG",    @"CPU Package Graphics", nil],
             [NSArray arrayWithObjects:@"PCPT",    @"CPU Package Total", nil],
             //[NSArray arrayWithObjects:@"PN0C",    @" ? ", nil],
             //[NSArray arrayWithObjects:@"PHPC",    @" ? ", nil],
             [NSArray arrayWithObjects:@"PB0R",    @"Battery Rail", nil],
             [NSArray arrayWithObjects:@"PZ0S",    @"Zone 1 Average", nil],
             
             nil], @"MacBookPro8,1",
            
            // iMac13,2
            [NSArray arrayWithObjects:
             // Temperatures
             [NSArray arrayWithObjects:@"TA0p",    @"Ambient", nil],
             [NSArray arrayWithObjects:@"TC:041c", @"CPU Core %X", nil],
             [NSArray arrayWithObjects:@"TC0p",    @"CPU Proximity", nil],
             [NSArray arrayWithObjects:@"TCXc",    @"PECI CPU", nil],
             [NSArray arrayWithObjects:@"TCSc",    @"PECI SA", nil],
             [NSArray arrayWithObjects:@"TCGc",    @"PECI GFX", nil],
             [NSArray arrayWithObjects:@"TPCD",    @"PCH Die", nil],
             [NSArray arrayWithObjects:@"Tm:021p", @"Mainboard Proximity %X", nil],
             [NSArray arrayWithObjects:@"TL:021p", @"LCD Proximity %X", nil],
             [NSArray arrayWithObjects:@"Tb0p",    @"BLC Proximity", nil],
             [NSArray arrayWithObjects:@"TM:141p", @"Memory Module %X", nil],
             [NSArray arrayWithObjects:@"TM:041p", @"Memory Module %X Proximity", nil],
             [NSArray arrayWithObjects:@"Tp2h",    @"Power Supply Heatsink", nil],
             
             // Voltages
             [NSArray arrayWithObjects:@"VC0C",     @"CPU Core", nil],
             [NSArray arrayWithObjects:@"VC0M",     @"CPU Memory", nil],
             [NSArray arrayWithObjects:@"VN1R",     @"PCH Core", nil],
             [NSArray arrayWithObjects:@"VR3R",     @"Main 3.3V", nil],
             [NSArray arrayWithObjects:@"VH05",     @"Main 5V", nil],
             [NSArray arrayWithObjects:@"VD2R",     @"Main 12V", nil],
             [NSArray arrayWithObjects:@"VG0C",     @"GPU Core", nil],

             
             // Currents
             [NSArray arrayWithObjects:@"IC0C",     @"CPU Core", nil],
             [NSArray arrayWithObjects:@"IC0M",     @"CPU Memory", nil],
             [NSArray arrayWithObjects:@"IN1R",     @"PCH Rail", nil],
             [NSArray arrayWithObjects:@"IM0R",     @"Memory Rail", nil],
             [NSArray arrayWithObjects:@"IH02",     @"Main 3.3V Rail", nil], // ??
             [NSArray arrayWithObjects:@"IH05",     @"Main 5V Rail", nil],
             [NSArray arrayWithObjects:@"ID2R",     @"Main 12V Rail", nil],
             [NSArray arrayWithObjects:@"IG0C",     @"GPU Rail", nil],
             
             // Powers
             [NSArray arrayWithObjects:@"PC0C",     @"CPU Core", nil],
             [NSArray arrayWithObjects:@"PC0M",     @"CPU Memory", nil],
             [NSArray arrayWithObjects:@"PCPC",     @"CPU Package Cores", nil],
             [NSArray arrayWithObjects:@"PCPG",     @"CPU Package Graphics", nil],
             [NSArray arrayWithObjects:@"PCPL",     @"CPU Package Total", nil],
             
             [NSArray arrayWithObjects:@"PN1R",     @"PCH Rail", nil], // ??
             
             [NSArray arrayWithObjects:@"PM0R",     @"Memory Rail", nil], // ??
             [NSArray arrayWithObjects:@"PH02",     @"Main 3.3V Rail", nil], // ??
             [NSArray arrayWithObjects:@"PH05",     @"Main 5V Rail", nil],
             [NSArray arrayWithObjects:@"PD2R",     @"Main 12V Rail", nil],
             
             [NSArray arrayWithObjects:@"PG0C",     @"GPU Core", nil],
             [NSArray arrayWithObjects:@"PGTR",     @"GPU Total", nil],
             
             [NSArray arrayWithObjects:@"PDTR",     @"System Total", nil],
             
             [NSArray arrayWithObjects:@"PZ:041G",  @"Zone %X Average", nil],

             nil], @"iMac13,2",
            
            // MacPro5,1
            [NSArray arrayWithObjects:
             // Temperatures
             [NSArray arrayWithObjects:@"TC:A2AC",    @"CPU %X Core", nil],
             [NSArray arrayWithObjects:@"TC:A2AD",    @"CPU %X Die", nil],
             [NSArray arrayWithObjects:@"TC:A2AH",    @"CPU %X Heatsink", nil],
             [NSArray arrayWithObjects:@"TN0D",       @"PCH Die", nil],
             [NSArray arrayWithObjects:@"TN0H",       @"PCH Heatsink", nil],
             [NSArray arrayWithObjects:@"TM:181P",    @"Memory Module %X Proximity", nil],
             [NSArray arrayWithObjects:@"TMA:141",    @"Memory Bank A, Slot %X", nil],
             [NSArray arrayWithObjects:@"TMB:141",    @"Memory Bank B, Slot %X", nil],
             [NSArray arrayWithObjects:@"TeGG",       @"PCIe GPU", nil],
             
             // Voltages
             [NSArray arrayWithObjects:@"VC:A2AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"VS2C",       @"CPU PLL", nil], // 1.82V
             [NSArray arrayWithObjects:@"VN0C",       @"CPU IMC/QPI/DRAM", nil], // 1.10V
             [NSArray arrayWithObjects:@"VM:A2AS",    @"Memory Bank %X", nil],
             [NSArray arrayWithObjects:@"VS1C",       @"PCH", nil], // 1.05V
             [NSArray arrayWithObjects:@"VS8C",       @"Main 3.3V", nil],
             [NSArray arrayWithObjects:@"Vp0C",       @"Power Supply 12V", nil],
             
             // Currents
             [NSArray arrayWithObjects:@"IC:A2AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"IM:A2AS",    @"Memory Bank %X", nil],
             [NSArray arrayWithObjects:@"IN0C",       @"PCH", nil],
             [NSArray arrayWithObjects:@"Ie:141S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"Ie:A2AS",    @"PCIe Booster %X", nil],
             [NSArray arrayWithObjects:@"Ip0C",       @"Power Supply 12V", nil],
             
             // Powers
             [NSArray arrayWithObjects:@"PC:A2AC",    @"CPU %X", nil],
             [NSArray arrayWithObjects:@"PM:A2AS",    @"Memory Bank %X", nil],
             [NSArray arrayWithObjects:@"PN0C",       @"PCH Core", nil],
             [NSArray arrayWithObjects:@"Pe:141S",    @"PCIe Slot %X", nil],
             [NSArray arrayWithObjects:@"Pe:A2AS",    @"PCIe Booster %X", nil],
             [NSArray arrayWithObjects:@"PpSM",       @"Memory Modules", nil],
             [NSArray arrayWithObjects:@"Pp0C",       @"Power Supply 12V", nil],
             
             nil], @"MacPro5,1",
            
            nil];
}

@end
