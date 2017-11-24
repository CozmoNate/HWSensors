/*
 *  HWSensors.h
 *  CPUSensorsPlugin
 *  
 *  Based on code by mercurysquad, superhai (C) 2008
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *  Based on code by slice (C) 2013
 *
 *  Created by kozlek on 30/09/10.
 *  Copyright 2010 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 */

/*
 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1
 
 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS" basis,
 WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 for the specific language governing rights and limitations under the License.
 
 The Original Code is the Open Hardware Monitor code.
 
 The Initial Developer of the Original Code is 
 Michael Möller <m.moeller@gmx.ch>.
 Portions created by the Initial Developer are Copyright (C) 2011
 the Initial Developer. All Rights Reserved.
 
 Contributor(s):
 
 Alternatively, the contents of this file may be used under the terms of
 either the GNU General Public License Version 2 or later (the "GPL"), or
 the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 in which case the provisions of the GPL or the LGPL are applicable instead
 of those above. If you wish to allow use of your version of this file only
 under the terms of either the GPL or the LGPL, and not to allow others to
 use your version of this file under the terms of the MPL, indicate your
 decision by deleting the provisions above and replace them with the notice
 and other provisions required by the GPL or the LGPL. If you do not delete
 the provisions above, a recipient may use your version of this file under
 the terms of any one of the MPL, the GPL or the LGPL.
 
 */

#include "CPUSensors.h"
#include "IntelDefinitions.h"

#include "timer.h"
#include "smc.h"

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IORegistryEntry.h>

enum {
    kCPUSensorsThermalCore              = BIT(0),
    kCPUSensorsThermalPackage           = BIT(1),
    kCPUSensorsMultiplierCore           = BIT(2),
    kCPUSensorsMultiplierPackage        = BIT(3),
    kCPUSensorsFrequencyCore            = BIT(4),
    kCPUSensorsFrequencyPackage         = BIT(5),
    kCPUSensorsFrequencyCoreAverage     = BIT(6),
    kCPUSensorsFrequencyPackageAverage  = BIT(7),


    kCPUSensorsPowerTotal               = BIT(8),
    kCPUSensorsPowerCores               = BIT(9),
    kCPUSensorsPowerUncore              = BIT(10),
    kCPUSensorsPowerDram                = BIT(11),

    kCPUSensorsVoltageCore              = BIT(12),
    kCPUSensorsVoltagePackage           = BIT(13),
};

static UInt16 cpu_energy_msrs[] =
{
    MSR_PKG_ENERY_STATUS,
    MSR_PP0_ENERY_STATUS,
    MSR_PP1_ENERY_STATUS,
    MSR_DRAM_ENERGY_STATUS
};

static UInt16 cpu_energy_flgs[] =
{
    kCPUSensorsPowerTotal,
    kCPUSensorsPowerCores,
    kCPUSensorsPowerUncore,
    kCPUSensorsPowerDram
};

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(CPUSensors, FakeSMCPlugin)

inline uint64_t rdmpc64(uint32_t counter)
{
    UInt32 lo,hi;
    rdpmc(counter, lo, hi);
    return ((UInt64)hi << 32 ) | lo;
}

//static inline UInt8 get_hex_index(char c)
//{       
//	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
//};

static inline UInt8 get_cpu_number()
{
    UInt8 number = cpu_number() & 0xFF;
    
    if (cpuid_info()->thread_count > cpuid_info()->core_count) {
        return !(number % 2) ? number >> 1 : UINT8_MAX;
    }
    
    return number;
}

static UInt8  cpu_tjmax[kCPUSensorsMaxCpus];

static void read_cpu_tjmax(void *magic)
{
    UInt32 number = get_cpu_number();

    if (number < kCPUSensorsMaxCpus) {
        cpu_tjmax[number] = (rdmsr64(MSR_IA32_TEMP_TARGET) >> 16) & 0xFF;
    }
}

static UInt64 cpu_rapl;

static void read_cpu_rapl(void *magic)
{
    UInt32 number = get_cpu_number();

    if (number == 0) {
        cpu_rapl = rdmsr64(MSR_RAPL_POWER_UNIT);
    }
}

static void update_counters(void *arg)
{
    CPUSensorsCounters *counters = (CPUSensorsCounters *)arg;

    UInt32 number = get_cpu_number();

    if (number < kCPUSensorsMaxCpus) {

        UInt64 msr;

        if (bit_get(counters->event_flags, kCPUSensorsThermalCore)) {
            if ((msr = rdmsr64(MSR_IA32_THERM_STS)) & 0x80000000) {
                counters->thermal_status[number] = (msr >> 16) & 0x7F;
            }
        }

        if (number == 0 && bit_get(counters->event_flags, kCPUSensorsThermalPackage)) {
            if ((msr = rdmsr64(MSR_IA32_PACKAGE_THERM_STATUS)) & 0x80000000) {
                counters->thermal_status_package = (msr >> 16) & 0x7F;
            }
        }

        if (bit_get(counters->event_flags, kCPUSensorsMultiplierCore) || (number == 0 && bit_get(counters->event_flags, kCPUSensorsMultiplierPackage))) {

            counters->perf_status[number] =  rdmsr64(MSR_IA32_PERF_STS) & 0xFFFF;

            // Performance counters
            if (counters->update_perf_counters) {
                counters->aperf_before[number] = counters->aperf_after[number];
                counters->mperf_before[number] = counters->mperf_after[number];
                counters->aperf_after[number] = rdmsr64(MSR_IA32_APERF);
                counters->mperf_after[number] = rdmsr64(MSR_IA32_MPERF);
            }
        }

        // Frequency counters
        if (counters->update_perf_counters && (bit_get(counters->event_flags, kCPUSensorsFrequencyCore) || (number == 0 && bit_get(counters->event_flags, kCPUSensorsFrequencyPackage)))) {
            counters->utc_before[number] = counters->utc_after[number];
            counters->urc_before[number] = counters->urc_after[number];
            counters->utc_after[number] = rdmpc64(0x40000001);
            counters->urc_after[number] = rdmpc64(0x40000002);
        }

        // Energy counters
        if (number == 0) {
            for (UInt8 index = 0; index < 4; index++) {
                if (bit_get(counters->event_flags, cpu_energy_flgs[index])) {
                    counters->energy_before[index] = counters->energy_after[index];
                    counters->energy_after[index] = rdmsr64(cpu_energy_msrs[index]);
                }
            }
        }
    }
}

void CPUSensors::calculateMultiplier(UInt32 index)
{
    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_NEHALEM:
        case CPUFAMILY_INTEL_WESTMERE:
            if (baseMultiplier > 0 && ratio[index] > 1.0)
                multiplier[index] = ROUND(ratio[index] * (float)baseMultiplier);
            else
                multiplier[index] = (float)(counters.perf_status[index] & 0xFF);
            break;
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_BROADWELL:
        case CPUFAMILY_INTEL_SKYLAKE:
        case CPUFAMILY_INTEL_KABYLAKE:
            if (baseMultiplier > 0 && ratio[index] > 1.0)
                multiplier[index] = ROUND(ratio[index] * (float)baseMultiplier);
            else
                multiplier[index] = (float)((counters.perf_status[index] >> 8) & 0xFF);
            break;
        default: {
            UInt8 fid = (counters.perf_status[index] >> 8) & 0xFF;
            multiplier[index] = float((float)((fid & 0x1f)) + 0.5f * (float)((fid >> 6) & 1));
            break;
        }
    }
}

void CPUSensors::calculateVoltage(UInt32 index)
{
    UInt8 vid = counters.perf_status[index] & 0xFF;

    switch (cpuid_info()->cpuid_model) {
        case CPUID_MODEL_PENTIUM_M:
            voltage[index] = 700 + ((vid & 0x3F) << 4);
            break;
        case CPUID_MODEL_YONAH:
            voltage[index] =   (1425 + ((vid & 0x3F) * 25)) >> 1;
            break;
        case CPUID_MODEL_MEROM: //Conroe?!
            voltage[index] =  (1650 + ((vid & 0x3F) * 25)) >> 1;
            break;
        case CPUID_MODEL_PENRYN:
        case CPUID_MODEL_ATOM:
            voltage[index] =   (1500 - (((~vid & 0x3F) * 25) >> 1));
            break;

        default:
            return;
    }

    voltage[index] = voltage[index] / 1000.0f;
}

void CPUSensors::calculateTimedCounters()
{
    if (bit_get(counters.event_flags, kCPUSensorsMultiplierCore | kCPUSensorsMultiplierPackage)) {
        for (UInt8 index = 0; index < coreCount; index++) {
            if (baseMultiplier) {
                UInt64 aperf = counters.aperf_after[index] - counters.aperf_before[index];
                UInt64 mperf = counters.mperf_after[index] - counters.mperf_before[index];

                if (mperf) {
                    ratio[index] = (double)aperf / (double)mperf;
                }
            }

            calculateMultiplier(index);
        }
    }

    if (bit_get(counters.event_flags, kCPUSensorsVoltageCore | kCPUSensorsVoltagePackage)) {
        for (UInt8 index = 0; index < coreCount; index++) {

            UInt8 vid = counters.perf_status[index] & 0xFF;

            switch (cpuid_info()->cpuid_model) {
                case CPUID_MODEL_PENTIUM_M:
                    voltage[index] = (float)(700 + ((vid & 0x3F) << 4)) / 1000.0f;
                    break;
                case CPUID_MODEL_YONAH:
                    voltage[index] = (float)((1425 + ((vid & 0x3F) * 25)) >> 1) / 1000.0f;
                    break;
                case CPUID_MODEL_MEROM: //Conroe?!
                    voltage[index] = (float)((1650 + ((vid & 0x3F) * 25)) >> 1) / 1000.0f;
                    break;
                case CPUID_MODEL_PENRYN:
                case CPUID_MODEL_ATOM:
                    voltage[index] = (float)(1500 - (((~vid & 0x3F) * 25) >> 1)) / 1000.0f;
                    break;
                    
                default: break;
            }
        }
    }

    if (bit_get(counters.event_flags, kCPUSensorsFrequencyCore | kCPUSensorsFrequencyPackage)) {
        for (UInt8 index = 0; index < coreCount; index++) {
            if (baseMultiplier > 0) {
                UInt64 thread_clocks = counters.utc_after[index] < counters.utc_before[index] ? UINT64_MAX - counters.utc_before[index] + counters.utc_after[index] : counters.utc_after[index] - counters.utc_before[index];
                UInt64 ref_clocks = counters.urc_after[index] < counters.urc_before[index] ? UINT64_MAX - counters.urc_before[index] + counters.urc_after[index] : counters.urc_after[index] - counters.urc_before[index];

                if (ref_clocks) {
                    turbo[index] = (double)thread_clocks / (double)ref_clocks;
                }
            }
            else if (!bit_get(counters.event_flags, kCPUSensorsMultiplierCore | kCPUSensorsMultiplierPackage)) {
                calculateMultiplier(index);
            }
        }
    }

    if (timerEventDeltaTime && timerEventDeltaTime < 10.0f && bit_get(counters.event_flags, kCPUSensorsPowerTotal | kCPUSensorsPowerCores | kCPUSensorsPowerUncore | kCPUSensorsPowerDram)) {
        for (UInt8 index = 0; index < 4; index++) {

            UInt64 deltaEnergy = counters.energy_after[index] < counters.energy_before[index] ? UINT64_MAX - counters.energy_before[index] + counters.energy_after[index] : counters.energy_after[index] - counters.energy_before[index];

            energy[index] = (double)deltaEnergy / timerEventDeltaTime;
        }
    }
}

IOReturn CPUSensors::timerEventAction()
{
    if (counters.event_flags) {

        double time = ptimer_read_seconds();

        timerEventDeltaTime =  time - timerEventLastTime;
        timerEventLastTime = time;

        mp_rendezvous_no_intrs(update_counters, &counters);

        calculateTimedCounters();

        if (timerEventDeltaTime == 0 || timerEventDeltaTime > 10.0f) {
            timerEventScheduled = timerEventSource->setTimeoutMS(500) == kIOReturnSuccess ? true : false;
        }
        else {
            timerEventScheduled = false;
        }
    }
    
    return kIOReturnSuccess;
}

bool CPUSensors::willReadSensorValue(FakeSMCSensor *sensor, float *outValue)
{    
    UInt32 index = sensor->getIndex();

    bit_set(counters.event_flags, sensor->getGroup());

    switch (sensor->getGroup()) {
        case kCPUSensorsThermalCore:
            *outValue = cpu_tjmax[index] - counters.thermal_status[index];
            break;

        case kCPUSensorsThermalPackage:
            *outValue = cpu_tjmax[index] - counters.thermal_status_package;
            break;
            
        case kCPUSensorsMultiplierCore:
        case kCPUSensorsMultiplierPackage:
            *outValue = multiplier[index];
            break;

        case kCPUSensorsVoltageCore:
        case kCPUSensorsVoltagePackage:
            *outValue = voltage[index];
            break;
            
        case kCPUSensorsFrequencyCore:
        case kCPUSensorsFrequencyPackage:
            switch (cpuid_info()->cpuid_cpufamily) {
                case CPUFAMILY_INTEL_NEHALEM:
                case CPUFAMILY_INTEL_WESTMERE:
                case CPUFAMILY_INTEL_SANDYBRIDGE:
                case CPUFAMILY_INTEL_IVYBRIDGE:
                case CPUFAMILY_INTEL_HASWELL:
                case CPUFAMILY_INTEL_BROADWELL:
                case CPUFAMILY_INTEL_SKYLAKE:
                case CPUFAMILY_INTEL_KABYLAKE:
                    *outValue = multiplier[index] * (float)busClock;
                    break;

                default: {
                    *outValue = multiplier[index] * (float)busClock * ((counters.perf_status[index] & 0x8000) ? 0.5 : 1.0);
                    break;
                }
            }
            break;

        case kCPUSensorsFrequencyCoreAverage:
        case kCPUSensorsFrequencyPackageAverage:
            *outValue = turbo[index] * (float)busClock * (float)baseMultiplier;
            break;

        case kCPUSensorsPowerTotal:
        case kCPUSensorsPowerCores:
        case kCPUSensorsPowerUncore:
        case kCPUSensorsPowerDram:
            *outValue = energyUnits * energy[index];
            break;

        default:
            return false;
            
    }

    if (!timerEventScheduled) {
        timerEventScheduled = timerEventSource->setTimeoutMS(50) == kIOReturnSuccess ? true : false;
    }
    
    return true;
}

FakeSMCSensor *CPUSensors::addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    FakeSMCSensor *result = super::addSensorForKey(key, type, size, group, index);
    
    if (result) {
        bit_set(counters.event_flags, group);
    }
    
    return result;
}

bool CPUSensors::start(IOService *provider)
{
    if (!super::start(provider)) 
        return false;

    // Pre-checks
    
    cpuid_set_info();
	
	if (strcmp(cpuid_info()->cpuid_vendor, CPUID_VID_INTEL) != 0)	{
		HWSensorsFatalLog("no Intel processor found");
		return false;
	}
	
	if(!(cpuid_info()->cpuid_features & CPUID_FEATURE_MSR))	{
		HWSensorsFatalLog("processor does not support Model Specific Registers (MSR)");
		return false;
	}

    // Init timer

    if (IOWorkLoop *workloop = getWorkLoop()) {
        if (!(timerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &CPUSensors::timerEventAction)))) {
            HWSensorsFatalLog("Failed to initialize timer event source");
            return false;
        }

        if (kIOReturnSuccess != workloop->addEventSource(timerEventSource))
        {
            HWSensorsFatalLog("Failed to add timer event source into workloop");
            return false;
        }
    }
    else {
        HWSensorsFatalLog("Failed to obtain current workloop");
        return false;
    }

    // Configure
        
    if (OSDictionary *configuration = getConfigurationNode())
    {
        if (OSNumber* number = OSDynamicCast(OSNumber, configuration->getObject("Tjmax"))) {

            UInt8 userTjmax = number->unsigned8BitValue();
            
            if (userTjmax) {
                memset(cpu_tjmax, userTjmax, kCPUSensorsMaxCpus);
                HWSensorsInfoLog("force Tjmax value to %d", cpu_tjmax[0]);
            }
        }
        
        if (OSString* string = OSDynamicCast(OSString, configuration->getObject("PlatformString"))) {
            // User defined platform key (RPlt)
            if (string->getLength() > 0) {
                char p[9] = "\0\0\0\0\0\0\0\0";
                snprintf(p, 9, "%s", string->getCStringNoCopy());
                platform = OSData::withBytes(p, 8);
            }
        }
    }

    // Estimating Tjmax value if not set
    if (!cpu_tjmax[0]) {
		switch (cpuid_info()->cpuid_family)
		{
			case 0x06: 
				switch (cpuid_info()->cpuid_model) 
                {
                    case CPUID_MODEL_PENTIUM_M:
                        cpu_tjmax[0] = 100;
                        if (!platform) platform = OSData::withBytes("M70\0\0\0\0\0", 8);
                        break;
                            
                    case CPUID_MODEL_YONAH:
                        if (!platform) platform = OSData::withBytes("K22\0\0\0\0\0", 8);
                        cpu_tjmax[0] = 85;
                        break;
                        
                    case CPUID_MODEL_MEROM: // Intel Core (65nm)
                        if (!platform) platform = OSData::withBytes("M75\0\0\0\0\0", 8);
                        switch (cpuid_info()->cpuid_stepping)
                        {
                            case 0x02: // G0
                            case 0x0A:
                                cpu_tjmax[0] = 100;
                                break;
                                
                            case 0x06: // B2
                                switch (cpuid_info()->core_count) 
                                {
                                    case 2:
                                        cpu_tjmax[0] = 80;
                                        break;
                                    case 4:
                                        cpu_tjmax[0] = 90;
                                        break;
                                    default:
                                        cpu_tjmax[0] = 85;
                                        break;
                                }
                                //tjmax[0] = 80; 
                                break;

                            case 0x0B: // G0
                                cpu_tjmax[0] = 90;
                                break;
                                
                            case 0x0D: // M0
                                cpu_tjmax[0] = 85;
                                break;
                                
                            default:
                                cpu_tjmax[0] = 85;
                                break;
                                
                        } 
                        break;
                        
                    case CPUID_MODEL_PENRYN: // Intel Core (45nm)
                                             // Mobile CPU ?
                        if (!platform) platform = OSData::withBytes("M82\0\0\0\0\0", 8);
                        if (rdmsr64(0x17) & (1<<28))
                            cpu_tjmax[0] = 105;
                        else
                            cpu_tjmax[0] = 100;
                        break;
                        
                    case CPUID_MODEL_ATOM: // Intel Atom (45nm)
                        if (!platform) platform = OSData::withBytes("T9\0\0\0\0\0", 8);
                        switch (cpuid_info()->cpuid_stepping)
                        {
                            case 0x02: // C0
                                cpu_tjmax[0] = 90;
                                break;
                            case 0x0A: // A0, B0
                                cpu_tjmax[0] = 100;
                                break;
                            default:
                                cpu_tjmax[0] = 90;
                                break;
                        } 
                        break;
                        
                    case CPUID_MODEL_NEHALEM:
                    case CPUID_MODEL_FIELDS:
                    case CPUID_MODEL_DALES:
                    case CPUID_MODEL_DALES_32NM:
                    case CPUID_MODEL_WESTMERE:
                    case CPUID_MODEL_NEHALEM_EX:
                    case CPUID_MODEL_WESTMERE_EX:
                        if (!platform) platform = OSData::withBytes("k74\0\0\0\0\0", 8);
                        mp_rendezvous_no_intrs(read_cpu_tjmax, NULL);
                        break;
                        
                    case CPUID_MODEL_SANDYBRIDGE:
                    case CPUID_MODEL_JAKETOWN:
                        if (!platform) platform = OSData::withBytes("k62\0\0\0\0\0", 8);
                        mp_rendezvous_no_intrs(read_cpu_tjmax, NULL);
                        break;
                        
                    case CPUID_MODEL_IVYBRIDGE:
                    case CPUID_MODEL_IVYBRIDGE_EP:
                        if (!platform) platform = OSData::withBytes("d8\0\0\0\0\0\0", 8);
                        mp_rendezvous_no_intrs(read_cpu_tjmax, NULL);
                        break;
                    
                    case CPUID_MODEL_HASWELL_MB:
                    case CPUID_MODEL_HASWELL_ULT:
                    case CPUID_MODEL_HASWELL_ULX:
                    case CPUID_MODEL_BROADWELL_MB:
                    case CPUID_MODEL_BROADWELL_ULV:
                    case CPUID_MODEL_SKYLAKE_LT:
                    case CPUID_MODEL_KABYLAKE_U:
                        if (!platform) platform = OSData::withBytes("j43\0\0\0\0\0", 8); // TODO: got from macbookair6,2 need to check for other platforms
                        mp_rendezvous_no_intrs(read_cpu_tjmax, NULL);
                        break;

                    case CPUID_MODEL_HASWELL_DT:
                    case CPUID_MODEL_BROADWELL_DT:
                    case CPUID_MODEL_SKYLAKE_X:
                    case CPUID_MODEL_SKYLAKE_DT:
                    case CPUID_MODEL_KABYLAKE_S:
                        if (!platform) platform = OSData::withBytes("j45\0\0\0\0\0", 8); // TODO: got from macbookpro11,2 need to check for other platforms
                        mp_rendezvous_no_intrs(read_cpu_tjmax, NULL);
                        break;
                        
                    default:
                        HWSensorsWarningLog("found unsupported Intel processor, using default Tjmax");
                        cpu_tjmax[0] = 100;
                        break;
                }
                break;
                
            case 0x0F: 
                switch (cpuid_info()->cpuid_model) 
                {
                    case 0x00: // Pentium 4 (180nm)
                    case 0x01: // Pentium 4 (130nm)
                    case 0x02: // Pentium 4 (130nm)
                    case 0x03: // Pentium 4, Celeron D (90nm)
                    case 0x04: // Pentium 4, Pentium D, Celeron D (90nm)
                    case 0x06: // Pentium 4, Pentium D, Celeron D (65nm)
                        cpu_tjmax[0] = 100;
                        break;
                        
                    default:
                        HWSensorsWarningLog("found unsupported Intel processor, using default Tjmax");
                        cpu_tjmax[0] = 100;
                        break;
                }
                break;
				
			default:
				HWSensorsFatalLog("found unknown Intel processor family");
				return false;
		}

        // Setup Tjmax
        switch (cpuid_info()->cpuid_cpufamily) {
            case CPUFAMILY_INTEL_NEHALEM:
            case CPUFAMILY_INTEL_WESTMERE:
            case CPUFAMILY_INTEL_SANDYBRIDGE:
            case CPUFAMILY_INTEL_IVYBRIDGE:
            case CPUFAMILY_INTEL_HASWELL:
            case CPUFAMILY_INTEL_BROADWELL:
            case CPUFAMILY_INTEL_SKYLAKE:
            case CPUFAMILY_INTEL_KABYLAKE:
                break;

            default: {
                UInt8 calculatedTjmax = cpu_tjmax[0];
                memset(cpu_tjmax, calculatedTjmax, kCPUSensorsMaxCpus);
                break;
            }
        }
	}

    // bus clock
    busClock = 0;
    
    if (IORegistryEntry *regEntry = fromPath("/efi/platform", gIODTPlane))
        if (OSData *data = OSDynamicCast(OSData, regEntry->getProperty("FSBFrequency")))
            busClock = *((UInt64*) data->getBytesNoCopy()) / 1e6;
    
    if (busClock == 0)
        busClock = (gPEClockFrequencyInfo.bus_frequency_max_hz >> 2) / 1e6;
    
    HWSensorsInfoLog("CPU family 0x%x, model 0x%x, stepping 0x%x, cores %d, threads %d, TJmax %d", cpuid_info()->cpuid_family, cpuid_info()->cpuid_model, cpuid_info()->cpuid_stepping, cpuid_info()->core_count, cpuid_info()->thread_count, cpu_tjmax[0]);
    
//    mp_rendezvous_no_intrs(cpu_check, NULL);
//    
//    for (int count = 0; count < kCPUSensorsMaxCpus; count++) {
//        if (cpu_enabled[count]) {
//            HWSensorsInfoLog("CPU[%d] lapic=0x%llx value = 0x%llx", count, cpu_lapic[count], cpu_check_value[count]);
//        }        
//    }

    // platform keys
    if (platform) {
        HWSensorsInfoLog("setting platform keys to [%-8s]", (const char*)platform->getBytesNoCopy());
        
        if (/*!isKeyExists("RPlt") &&*/ !setKeyValue("RPlt", SMC_TYPE_CH8, platform->getLength(), (void*)platform->getBytesNoCopy()))
            HWSensorsWarningLog("failed to set platform key RPlt");
        
        if (/*!isKeyExists("RBr") &&*/ !setKeyValue("RBr", SMC_TYPE_CH8, platform->getLength(), (void*)platform->getBytesNoCopy()))
            HWSensorsWarningLog("failed to set platform key RBr");
    }
    
    HWSensorsDebugLog("adding digital thermal sensors at core level");

    bit_set(counters.event_flags, kCPUSensorsThermalCore);
    mp_rendezvous_no_intrs(update_counters, &counters);
                           
    for (uint32_t i = 0; i < kCPUSensorsMaxCpus; i++) {
        if (counters.thermal_status[i]) {
            
            coreCount++;
            
            char key[5];
            
            snprintf(key, 5, KEY_FORMAT_CPU_DIE_TEMPERATURE, i);
            
            if (!addSensor(key, SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kCPUSensorsThermalCore, i)) {
                HWSensorsWarningLog("failed to add temperature sensor");
            }
        }
    }
    
    HWSensorsDebugLog("adding digital thermal sensor at package level");

    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_BROADWELL:
        case CPUFAMILY_INTEL_SKYLAKE:
        case CPUFAMILY_INTEL_KABYLAKE:
        {
            uint32_t cpuid_reg[4];
            
            do_cpuid(6, cpuid_reg);
            
            if ((uint32_t)bitfield32(cpuid_reg[eax], 4, 4)) {
                if (!addSensor(KEY_CPU_PACKAGE_TEMPERATURE, SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kCPUSensorsThermalPackage, 0))
                    HWSensorsWarningLog("failed to add cpu package temperature sensor");
            }
            break;
        }
    }

    HWSensorsDebugLog("adding multiplier & frequency sensors");

    if ((baseMultiplier = (rdmsr64(MSR_PLATFORM_INFO) >> 8) & 0xFF)) {
        //mp_rendezvous_no_intrs(init_cpu_turbo_counters, NULL);
        HWSensorsInfoLog("base CPU multiplier is %d", baseMultiplier);
        counters.update_perf_counters = true;
    }

    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_BROADWELL:
        case CPUFAMILY_INTEL_SKYLAKE:
        case CPUFAMILY_INTEL_KABYLAKE:
            if (!addSensor(KEY_FAKESMC_CPU_PACKAGE_MULTIPLIER, SMC_TYPE_FP88, SMC_TYPE_FPXX_SIZE, kCPUSensorsMultiplierPackage, 0))
                HWSensorsWarningLog("failed to add package multiplier sensor");
            if (!addSensor(KEY_FAKESMC_CPU_PACKAGE_FREQUENCY, SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kCPUSensorsFrequencyPackage, 0))
                HWSensorsWarningLog("failed to add package frequency sensor");

        case CPUFAMILY_INTEL_NEHALEM:
        case CPUFAMILY_INTEL_WESTMERE:
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        default:
            for (uint32_t i = 0; i < cpuid_info()->core_count; i++) {
                char key[5];
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_CPU_MULTIPLIER, i);
                
                if (!addSensor(key, SMC_TYPE_FP88, SMC_TYPE_FPXX_SIZE, kCPUSensorsMultiplierCore, i))
                    HWSensorsWarningLog("failed to add multiplier sensor");
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_CPU_FREQUENCY, i);
                
                if (!addSensor(key, SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kCPUSensorsFrequencyCore, i))
                    HWSensorsWarningLog("failed to add frequency sensor");

            }
            break;
    }

    HWSensorsDebugLog("adding average frequency sensor");

    if (baseMultiplier) {
        if (!addSensor(KEY_FAKESMC_CPU_PACKAGE_FREQUENCY_AVERAGE, SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kCPUSensorsFrequencyPackageAverage, 0))
        HWSensorsWarningLog("failed to add package average frequency sensor");
    }

    HWSensorsDebugLog("adding voltage sensor");

    switch (cpuid_info()->cpuid_model) {
        case CPUID_MODEL_PENTIUM_M:
        case CPUID_MODEL_YONAH:
        case CPUID_MODEL_MEROM: //Conroe?!
        case CPUID_MODEL_PENRYN:
        case CPUID_MODEL_ATOM:
            if (!addSensor(KEY_CPU_VOLTAGE, "sp3c", SMC_TYPE_SPXX_SIZE, kCPUSensorsVoltagePackage, 0))
                HWSensorsWarningLog("failed to add voltage sensor");
            break;

        default:
            break;
    }

    HWSensorsDebugLog("adding energy consumption sensors");

    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_BROADWELL:
        case CPUFAMILY_INTEL_SKYLAKE:
        case CPUFAMILY_INTEL_KABYLAKE:
        {
            mp_rendezvous_no_intrs(read_cpu_rapl, NULL);

            UInt8 power_units = cpu_rapl & 0xf;
            UInt8 energy_units = (cpu_rapl >> 8) & 0x1f;
            UInt8 time_units = (cpu_rapl >> 16) & 0xf;
            
            HWSensorsDebugLog("RAPL units power: 0x%x energy: 0x%x time: 0x%x", power_units, energy_units, time_units);
            
            if (energy_units && (energyUnits = 1.0f / (float)(1 << energy_units))) {
                if (!addSensor(KEY_CPU_PACKAGE_TOTAL_POWER, SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kCPUSensorsPowerTotal, 0))
                    HWSensorsWarningLog("failed to add CPU package total power sensor");
                
                if (!addSensor(KEY_CPU_PACKAGE_CORE_POWER, SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kCPUSensorsPowerCores, 1))
                    HWSensorsWarningLog("failed to add CPU package cores power sensor");
                
                // Uncore sensor is only available on CPUs with uncore device (built-in GPU)
                switch (cpuid_info()->cpuid_model) {
                    case CPUID_MODEL_SANDYBRIDGE:
                    case CPUID_MODEL_NEHALEM_EX:
                    case CPUID_MODEL_IVYBRIDGE:
                    case CPUID_MODEL_HASWELL_DT:
                    case CPUID_MODEL_HASWELL_ULT:
                    case CPUID_MODEL_HASWELL_ULX:
                    case CPUID_MODEL_BROADWELL_DT:
                    case CPUID_MODEL_BROADWELL_ULV:
                    case CPUID_MODEL_SKYLAKE_LT:
                    case CPUID_MODEL_SKYLAKE_DT:
                        if (!addSensor(KEY_CPU_PACKAGE_GFX_POWER, SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kCPUSensorsPowerUncore, 2))
                            HWSensorsWarningLog("failed to add CPU package uncore power sensor");
                        break;

                    default:
                        // Do not add uncore sensor
                        break;
                }

                switch (cpuid_info()->cpuid_cpufamily) {
                    case CPUFAMILY_INTEL_HASWELL:
                    case CPUFAMILY_INTEL_BROADWELL:
                    case CPUFAMILY_INTEL_SKYLAKE:
                    case CPUFAMILY_INTEL_KABYLAKE:
                        // TODO: check DRAM availability for other platforms
                        if (!addSensor(KEY_CPU_PACKAGE_DRAM_POWER, SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kCPUSensorsPowerDram, 3))
                            HWSensorsWarningLog("failed to add CPU package DRAM power sensor");
                        break;

                    default:
                        break;
                }
            }
            break;
        }
            
    }
    
    // two power states - off and on
	static const IOPMPowerState powerStates[2] = {
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, IOPMDeviceUsable, IOPMPowerOn, IOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    // register interest in power state changes
	PMinit();
	provider->joinPMtree(this);
	registerPowerDriver(this, (IOPMPowerState *)powerStates, 2);

    // Register service
    registerService();

    timerEventSource->setTimeoutMS(1000);

    HWSensorsInfoLog("started");

    return true;
}

IOReturn CPUSensors::setPowerState(unsigned long powerState, IOService *device)
{
    void *magic;

	switch (powerState) {
        case 0: // Power Off
                //timerEventSource->cancelTimeout();
            break;

        case 1: // Power On
                //timerEventSource->setTimeoutMS(1000);
            if (baseMultiplier > 0) {
                mp_rendezvous_no_intrs(init_cpu_turbo_counters, &magic);
            }
            break;

        default:
            break;
    }

	return(IOPMAckImplied);
}

void CPUSensors::stop(IOService *provider)
{
    PMstop();
    
    timerEventSource->cancelTimeout();

    if (IOWorkLoop *workloop = getWorkLoop()) {
        workloop->removeEventSource(timerEventSource);
    }
    
    super::stop(provider);
}

void CPUSensors::free()
{
    super::free();
}
