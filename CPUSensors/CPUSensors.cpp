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
#include "FakeSMCDefinitions.h"
#include "IntelDefinitions.h"

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IORegistryEntry.h>

#include "timer.h"

enum {
    kCPUSensorsCoreThermalSensor           = BIT(0),
    kCPUSensorsPackageThermalSensor        = BIT(1),
    kCPUSensorsCoreMultiplierSensor        = BIT(2),
    kCPUSensorsPackageMultiplierSensor     = BIT(3),
    kCPUSensorsCoreFrequencySensor         = BIT(4),
    kCPUSensorsPackageFrequencySensor      = BIT(5),
    kCPUSensorsTotalPowerSensor            = BIT(6),
    kCPUSensorsCoresPowerSensor            = BIT(7),
    kCPUSensorsUncorePowerSensor           = BIT(8),
    kCPUSensorsDramPowerSensor             = BIT(9),
};

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(CPUSensors, FakeSMCPlugin)

static inline UInt8 get_hex_index(char c)
{       
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
};

static inline UInt8 get_cpu_number()
{
    UInt8 number = cpu_number() & 0xFF;
    
    if (cpuid_info()->thread_count > cpuid_info()->core_count) {
        return !(number % 2) ? number >> 1 : UINT8_MAX;
    }
    
    return number;
}

static bool cpu_enabled[kCPUSensorsMaxCpus];
static UInt64 cpu_lapic[kCPUSensorsMaxCpus];
static UInt64 cpu_check_value[kCPUSensorsMaxCpus];

static inline void cpu_check(void *magic)
{
    int number = cpu_number();
    
    if (number < kCPUSensorsMaxCpus) {
        
        
        cpu_enabled[number] = true;
        
        uint32_t cpuid_reg[4];
        
        do_cpuid(0x0b, cpuid_reg);
        
        cpu_lapic[number] = cpuid_reg[edx];
        cpu_check_value[number] = rdmsr64(MSR_IA32_PACKAGE_THERM_STATUS);
    }
}

static UInt8 cpu_thermal[kCPUSensorsMaxCpus];
static UInt8 cpu_thermal_updated[kCPUSensorsMaxCpus];
static UInt8 cpu_thermal_package;

static inline void read_cpu_thermal(void *magic)
{
    UInt32 number = get_cpu_number();
    
    if (number < kCPUSensorsMaxCpus) {
        UInt64 msr = rdmsr64(MSR_IA32_THERM_STS);
        if (msr & 0x80000000) {
            cpu_thermal[number] = (msr >> 16) & 0x7F;
            cpu_thermal_updated[number] = true;
        }
    }
}

static UInt16 cpu_state[kCPUSensorsMaxCpus];
static bool cpu_state_updated[kCPUSensorsMaxCpus];

static inline void read_cpu_state(void *index)
{
    UInt32 number = get_cpu_number();
    
    if (index && *(UInt32*)index != number)
        return;
    
    if (number < kCPUSensorsMaxCpus) {
        cpu_state[number] = rdmsr64(MSR_IA32_PERF_STS) & 0xFFFF;
        cpu_state_updated[number] = true;
    }
}

static float cpu_turbo[kCPUSensorsMaxCpus];

static inline void read_cpu_turbo(void *index)
{
    UInt32 number = get_cpu_number();
    
    if (index && *(UInt32*)index != number)
        return;
    
    if (number < kCPUSensorsMaxCpus) {
        UInt64 utc_before = rdmsr64(MSR_CPU_CLK_UNHALTED_THREAD_ADDR);
        UInt64 urc_before = rdmsr64(MSR_CPU_CLK_UNHALTED_REF_ADDR);
        //UInt64 tsc_before = rdmsr64(MSR_IA32_TIME_STAMP_COUNTER);
        
        IODelay(500);
        
        UInt64 utc_after = rdmsr64(MSR_CPU_CLK_UNHALTED_THREAD_ADDR);
        UInt64 urc_after = rdmsr64(MSR_CPU_CLK_UNHALTED_REF_ADDR);
        //UInt64 tsc_after = rdmsr64(MSR_IA32_TIME_STAMP_COUNTER);
        
        double utc_delta = utc_after < utc_before ? UINT64_MAX - utc_before + utc_after : utc_after - utc_before;
        double urc_delta = urc_after < urc_before ? UINT64_MAX - urc_before + urc_after : urc_after - urc_before;
        //double tsc_delta = tsc_after < tsc_before ? UINT64_MAX - tsc_before + tsc_after : tsc_after - tsc_before;
        
        if (utc_delta && urc_delta /*&& tsc_delta*/) {
            cpu_turbo[number] = (float)(utc_delta / urc_delta) /** (float)(urc_delta / tsc_delta)*/;
            cpu_state_updated[number] = true;
        }
    }
}

static float cpu_ratio[kCPUSensorsMaxCpus];

static inline void read_cpu_ratio(void *index)
{
    UInt32 number = get_cpu_number();
    
    if (index && *(UInt32*)index != number)
        return;
    
    if (number < kCPUSensorsMaxCpus) {
        UInt64 MPERF = rdmsr64(MSR_IA32_MPERF);
        UInt64 APERF = rdmsr64(MSR_IA32_APERF);
        
        if (APERF && MPERF) {
            cpu_ratio[number] = (float)((double)APERF / (double)MPERF);
            cpu_state_updated[number] = true;
            
            wrmsr64(MSR_IA32_APERF, 0);
            wrmsr64(MSR_IA32_MPERF, 0);
        }
    }
}

static float cpu_energy_delta[4];
static double cpu_energy_last_time[4];
static UInt64 cpu_energy_last_value[4];

inline void read_cpu_energy(void *idx)
{
    UInt8 *index = (UInt8*)idx;
    
    double time = ptimer_read_seconds();
    UInt64 energy = rdmsr64(cpu_energy_msrs[*index]);
    
    if (!energy || energy < cpu_energy_last_value[*index]) {
        cpu_energy_last_time[*index] = time;
        cpu_energy_last_value[*index] = energy;
        return;
    }
    
    float deltaTime = float(time - cpu_energy_last_time[*index]);
    
    if (deltaTime < 1 || deltaTime > 30) {
        cpu_energy_last_time[*index] = time;
        cpu_energy_last_value[*index] = energy;
        return;
    }
    
    cpu_energy_delta[*index] = float(energy - cpu_energy_last_value[*index]) / deltaTime;
    
    cpu_energy_last_time[*index] = time;
    cpu_energy_last_value[*index] = energy;
}

void CPUSensors::readTjmaxFromMSR()
{
	for (uint32_t i = 0; i < cpuid_info()->core_count; i++) {
		tjmax[i] = (rdmsr64(MSR_IA32_TEMP_TARGET) >> 16) & 0xFF;
	}
}

#define ROUND(x)    ((x) + 0.5 > int(x) + 1 ? int(x) + 1 : int(x))

float CPUSensors::getSensorValue(FakeSMCSensor *sensor)
{
    //IOSimpleLockLock(workloopLock);
    
    UInt32 index = sensor->getIndex();
    
    switch (sensor->getGroup()) {
        case kCPUSensorsCoreThermalSensor:
            if (!cpu_thermal_updated[index]) {
                bit_set(timerEventsPending, kCPUSensorsCoreThermalSensor);
            }
            cpu_thermal_updated[index] = false;
            return tjmax[index] - cpu_thermal[index];
            
        case kCPUSensorsPackageThermalSensor:
            bit_set(timerEventsPending, kCPUSensorsPackageThermalSensor);
            return float(tjmax[0] - cpu_thermal_package);
            
        case kCPUSensorsCoreMultiplierSensor:
        case kCPUSensorsPackageMultiplierSensor:
            if (!cpu_state_updated[index]) {
                bit_set(timerEventsPending, sensor->getGroup());
            }
            cpu_state_updated[index] = false;
            switch (cpuid_info()->cpuid_cpufamily) {
                case CPUFAMILY_INTEL_NEHALEM:
                case CPUFAMILY_INTEL_WESTMERE:
                    if (baseMultiplier > 0 && cpu_ratio[index] > 1.0)
                        multiplier[index] = ROUND(cpu_ratio[index] * (float)baseMultiplier);
                    else
                        multiplier[index] = (float)(cpu_state[index] & 0xFF);
                    break;
                case CPUFAMILY_INTEL_SANDYBRIDGE:
                case CPUFAMILY_INTEL_IVYBRIDGE:
                case CPUFAMILY_INTEL_HASWELL:
                case CPUFAMILY_INTEL_HASWELL_ULT:
                    if (baseMultiplier > 0 && cpu_ratio[index] > 1.0)
                        multiplier[index] = ROUND(cpu_ratio[index] * (float)baseMultiplier);
                    else
                        multiplier[index] = (float)((cpu_state[index] >> 8) & 0xFF);
                    break;
                default: {
                    UInt8 fid = (cpu_state[0] >> 8) & 0xFF;
                    multiplier[index] = float((float)((fid & 0x1f)) * (fid & 0x80 ? 0.5 : 1.0) + 0.5f * (float)((fid >> 6) & 1));
                    break;
                }
            }
            return multiplier[index];
            
        case kCPUSensorsCoreFrequencySensor:
        case kCPUSensorsPackageFrequencySensor:
            return multiplier[index] * (float)busClock;
            
        case kCPUSensorsTotalPowerSensor:
        case kCPUSensorsCoresPowerSensor:
        case kCPUSensorsUncorePowerSensor:
        case kCPUSensorsDramPowerSensor:
            bit_set(timerEventsPending, sensor->getGroup());
            return (float)energyUnits * cpu_energy_delta[index];
            
    }
    
    //IOSimpleLockUnlock(workloopLock);
    
    return 0;
}

IOReturn CPUSensors::woorkloopTimerEvent()
{
    if (timerEventsPending) {
        if (++timerEventsMomentum > 5) {
            timerEventsMomentum = 0;
            timerEventsPending = 0;
        }
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsCoreThermalSensor)) {
        mp_rendezvous_no_intrs(read_cpu_thermal, NULL);
        //bit_clear(timerEventsPending, kCPUSensorsCoreThermalSensor);
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsPackageThermalSensor)) {
        cpu_thermal_package = ((rdmsr64(MSR_IA32_PACKAGE_THERM_STATUS) >> 16) & 0x7F);
        //bit_clear(timerEventsPending, kCPUSensorsCoreThermalSensor);
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsCoreMultiplierSensor)) {
        if (baseMultiplier > 0)
            mp_rendezvous_no_intrs(read_cpu_ratio, NULL);
            //mp_rendezvous_no_intrs(read_cpu_turbo, NULL);
        //else
            mp_rendezvous_no_intrs(read_cpu_state, NULL);
        //bit_clear(timerEventsPending, kCPUSensorsCoreMultiplierSensor);
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsPackageMultiplierSensor)) {
        UInt32 index = 0;
        if (baseMultiplier > 0)
            mp_rendezvous_no_intrs(read_cpu_ratio, NULL);
            //mp_rendezvous_no_intrs(read_cpu_turbo, &index);
        //else
        if (cpu_ratio[index] <= 1.0f)
            mp_rendezvous_no_intrs(read_cpu_state, &index);
        //bit_clear(timerEventsPending, kCPUSensorsPackageMultiplierSensor);
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsTotalPowerSensor)) {
        UInt8 index = 0;
        read_cpu_energy(&index);
        //bit_clear(timerEventsPending, kCPUSensorsTotalPowerSensor);
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsCoresPowerSensor)) {
        UInt8 index = 1;
        read_cpu_energy(&index);
        //bit_clear(timerEventsPending, kCPUSensorsCoresPowerSensor);
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsUncorePowerSensor)) {
        UInt8 index = 2;
        read_cpu_energy(&index);
        //bit_clear(timerEventsPending, kCPUSensorsUncorePowerSensor);
    }
    
    if (bit_get(timerEventsPending, kCPUSensorsDramPowerSensor)) {
        UInt8 index = 3;
        read_cpu_energy(&index);
        //bit_clear(timerEventsPending, kCPUSensorsDramPowerSensor);
    }
    
    timerEventSource->setTimeoutMS(1000);
    
    //IOSimpleLockUnlock(workloopLock);
    
    return kIOReturnSuccess;
}

FakeSMCSensor *CPUSensors::addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    FakeSMCSensor *result = super::addSensor(key, type, size, group, index);
    
    if (result) {
        bit_set(timerEventsPending, group);
    }
    
    return result;
}

bool CPUSensors::start(IOService *provider)
{
    if (!super::start(provider)) 
        return false;
    
    cpuid_update_generic_info();
	
	if (strcmp(cpuid_info()->cpuid_vendor, CPUID_VID_INTEL) != 0)	{
		HWSensorsFatalLog("no Intel processor found");
		return false;
	}
	
	if(!(cpuid_info()->cpuid_features & CPUID_FEATURE_MSR))	{
		HWSensorsFatalLog("processor does not support Model Specific Registers (MSR)");
		return false;
	}
    
	if(cpuid_info()->core_count == 0)	{
		HWSensorsFatalLog("CPU core count is zero");
		return false;
	}
        
    if (OSDictionary *configuration = getConfigurationNode())
    {
        if (OSNumber* number = OSDynamicCast(OSNumber, configuration->getObject("Tjmax"))) {
            // User defined Tjmax
            tjmax[0] = number->unsigned32BitValue();
            
            if (tjmax[0] > 0) {
                for (uint32_t i = 1; i < cpuid_info()->core_count; i++)
                    tjmax[i] = tjmax[0];
                
                HWSensorsInfoLog("force Tjmax value to %d", tjmax[0]);
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
    
    if (tjmax[0] == 0) {
		// Calculating Tjmax
		switch (cpuid_info()->cpuid_family)
		{
			case 0x06: 
				switch (cpuid_info()->cpuid_model) 
                {
                    case CPUID_MODEL_PENTIUM_M:
                        tjmax[0] = 100;
                        if (!platform) platform = OSData::withBytes("M70\0\0\0\0\0", 8);
                        break;
                            
                    case CPUID_MODEL_YONAH:
                        if (!platform) platform = OSData::withBytes("K22\0\0\0\0\0", 8);
                        tjmax[0] = 85;
                        break;
                        
                    case CPUID_MODEL_MEROM: // Intel Core (65nm)
                        if (!platform) platform = OSData::withBytes("M75\0\0\0\0\0", 8);
                        switch (cpuid_info()->cpuid_stepping)
                        {
                            case 0x02: // G0
                                tjmax[0] = 100; 
                                break;
                                
                            case 0x06: // B2
                                switch (cpuid_info()->core_count) 
                                {
                                    case 2:
                                        tjmax[0] = 80; 
                                        break;
                                    case 4:
                                        tjmax[0] = 90; 
                                        break;
                                    default:
                                        tjmax[0] = 85; 
                                        break;
                                }
                                //tjmax[0] = 80; 
                                break;
                                
                            case 0x0B: // G0
                                tjmax[0] = 90; 
                                break;
                                
                            case 0x0D: // M0
                                tjmax[0] = 85; 
                                break;
                                
                            default:
                                tjmax[0] = 85; 
                                break;
                                
                        } 
                        break;
                        
                    case CPUID_MODEL_PENRYN: // Intel Core (45nm)
                                             // Mobile CPU ?
                        if (!platform) platform = OSData::withBytes("M82\0\0\0\0\0", 8);
                        if (rdmsr64(0x17) & (1<<28))
                            tjmax[0] = 105;
                        else
                            tjmax[0] = 100; 
                        break;
                        
                    case CPUID_MODEL_ATOM: // Intel Atom (45nm)
                        if (!platform) platform = OSData::withBytes("T9\0\0\0\0\0", 8);
                        switch (cpuid_info()->cpuid_stepping)
                        {
                            case 0x02: // C0
                                tjmax[0] = 90; 
                                break;
                            case 0x0A: // A0, B0
                                tjmax[0] = 100; 
                                break;
                            default:
                                tjmax[0] = 90; 
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
                        readTjmaxFromMSR();
                        break;
                        
                    case CPUID_MODEL_SANDYBRIDGE:
                    case CPUID_MODEL_JAKETOWN:
                        if (!platform) platform = OSData::withBytes("k62\0\0\0\0\0", 8);
                        readTjmaxFromMSR();
                        break;
                        
                    case CPUID_MODEL_IVYBRIDGE:
                        if (!platform) platform = OSData::withBytes("d8\0\0\0\0\0\0", 8);
                        readTjmaxFromMSR();
                        break;
                    
                    case CPUID_MODEL_HASWELL_DT:
                    case CPUID_MODEL_HASWELL_MB:
                        // TODO: platform value for desktop Haswells
                    case CPUID_MODEL_HASWELL_ULT:
                    case CPUID_MODEL_HASWELL_ULX:
                        if (!platform) platform = OSData::withBytes("j43\0\0\0\0\0", 8); // TODO: got from macbookair6,2 need to check for other platforms
                        readTjmaxFromMSR();
                        break;
                        
                    default:
                        HWSensorsFatalLog("found unsupported Intel processor, using default Tjmax");
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
                        tjmax[0] = 100;
                        break;
                        
                    default:
                        HWSensorsFatalLog("found unsupported Intel processor, using default Tjmax");
                        break;
                }
                break;
				
			default:
				HWSensorsFatalLog("found unknown Intel processor family");
				return false;
		}
	}
	
    // Setup Tjmax
    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_NEHALEM:
        case CPUFAMILY_INTEL_WESTMERE:
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_HASWELL_ULT:
            break;
            
        default:
            for (uint32_t i = 1; i < cpuid_info()->core_count; i++)
                tjmax[i] = tjmax[0];
            break;
    }
    
    // woorkloop
    if (!(workloop = getWorkLoop())) {
        HWSensorsFatalLog("Failed to obtain workloop");
        return false;
    }
    
    if (!(timerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &CPUSensors::woorkloopTimerEvent)))) {
        HWSensorsFatalLog("Failed to initialize timer event source");
        return false;
    }
    
    if (kIOReturnSuccess != workloop->addEventSource(timerEventSource))
    {
        HWSensorsFatalLog("Failed to add timer event source into workloop");
        return false;
    }
    
    //workloopLock = IOSimpleLockAlloc();
    
    // bus clock
    busClock = 0;
    
    if (IORegistryEntry *regEntry = fromPath("/efi/platform", gIODTPlane))
        if (OSData *data = OSDynamicCast(OSData, regEntry->getProperty("FSBFrequency")))
            busClock = *((UInt64*) data->getBytesNoCopy()) / 1e6;
    
    if (busClock == 0)
        busClock = (gPEClockFrequencyInfo.bus_frequency_max_hz >> 2) / 1e6;
    
    HWSensorsInfoLog("CPU family 0x%x, model 0x%x, stepping 0x%x, cores %d, threads %d, TJmax %d", cpuid_info()->cpuid_family, cpuid_info()->cpuid_model, cpuid_info()->cpuid_stepping, cpuid_info()->core_count, cpuid_info()->thread_count, tjmax[0]);
    
//    mp_rendezvous_no_intrs(cpu_check, NULL);
//    
//    for (int count = 0; count < kCPUSensorsMaxCpus; count++) {
//        if (cpu_enabled[count]) {
//            HWSensorsInfoLog("CPU[%d] lapic=0x%llx value = 0x%llx", count, cpu_lapic[count], cpu_check_value[count]);
//        }        
//    }
    
    enableExclusiveAccessMode();
    
    // platform keys
    if (platform) {
        HWSensorsInfoLog("setting platform keys to [%-8s]", (const char*)platform->getBytesNoCopy());
        
        if (/*!isKeyExists("RPlt") &&*/ !setKeyValue("RPlt", TYPE_CH8, platform->getLength(), (void*)platform->getBytesNoCopy()))
            HWSensorsWarningLog("failed to set platform key RPlt");
        
        if (/*!isKeyExists("RBr") &&*/ !setKeyValue("RBr", TYPE_CH8, platform->getLength(), (void*)platform->getBytesNoCopy()))
            HWSensorsWarningLog("failed to set platform key RBr");
    }
    
    // digital thermal sensor at core level

    uint32_t available_cores_count = 0;
    
    mp_rendezvous_no_intrs(read_cpu_thermal, NULL);
                           
    for (uint32_t i = 0; i < kCPUSensorsMaxCpus; i++) {
        if (cpu_thermal[i]) {
            
            available_cores_count++;
            
            char key[5];
            
            snprintf(key, 5, KEY_FORMAT_CPU_DIE_TEMPERATURE, i);
            
            if (!addSensor(key, TYPE_SP78, TYPE_SPXX_SIZE, kCPUSensorsCoreThermalSensor, i))
                HWSensorsWarningLog("failed to add temperature sensor");
        }
    }
    
    // digital thermal sensor at package level
    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_HASWELL_ULT: {
            uint32_t cpuid_reg[4];
            
            do_cpuid(6, cpuid_reg);
            
            if ((uint32_t)bitfield(cpuid_reg[eax], 4, 4)) {
                if (!addSensor(KEY_CPU_PACKAGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kCPUSensorsPackageThermalSensor, 0))
                    HWSensorsWarningLog("failed to add cpu package temperature sensor");
            }
            break;
        }
    }
    
    // multiplier
    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
            if ((baseMultiplier = (rdmsr64(MSR_PLATFORM_INFO) >> 8) & 0xFF)) {
                //mp_rendezvous_no_intrs(init_cpu_turbo_counters, NULL);
                HWSensorsInfoLog("base CPU multiplier is %d", baseMultiplier);
            }
        // break; fall down adding package sensors
            
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_HASWELL_ULT:
            // 
            if (!addSensor(KEY_FAKESMC_CPU_PACKAGE_MULTIPLIER, TYPE_FP88, TYPE_FPXX_SIZE, kCPUSensorsPackageMultiplierSensor, 0))
                HWSensorsWarningLog("failed to add package multiplier sensor");
            if (!addSensor(KEY_FAKESMC_CPU_PACKAGE_FREQUENCY, TYPE_UI32, TYPE_UI32_SIZE, kCPUSensorsPackageFrequencySensor, 0))
                HWSensorsWarningLog("failed to add package frequency sensor");
            
            break;
            
        case CPUFAMILY_INTEL_NEHALEM:
        case CPUFAMILY_INTEL_WESTMERE:
            if ((baseMultiplier = (rdmsr64(MSR_PLATFORM_INFO) >> 8) & 0xFF))
                HWSensorsInfoLog("base CPU multiplier is %d", baseMultiplier);
            // break; fall down adding multiplier sensors for each core
        default:
            for (uint32_t i = 0; i < available_cores_count/*cpuid_info()->core_count*/; i++) {
                char key[5];
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_CPU_MULTIPLIER, i);
                
                if (!addSensor(key, TYPE_FP88, TYPE_FPXX_SIZE, kCPUSensorsCoreMultiplierSensor, i))
                    HWSensorsWarningLog("failed to add multiplier sensor");
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_CPU_FREQUENCY, i);
                
                if (!addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kCPUSensorsCoreFrequencySensor, i))
                    HWSensorsWarningLog("failed to add frequency sensor");
                
            }
            break;
    }
    
    // energy consumption
    switch (cpuid_info()->cpuid_cpufamily) {            
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_HASWELL_ULT: {
            UInt64 rapl = rdmsr64(MSR_RAPL_POWER_UNIT);
            
            UInt8 power_units = rapl & 0xf;
            UInt8 energy_units = (rapl >> 8) & 0x1f;
            UInt8 time_units = (rapl >> 16) & 0xf;
            
            HWSensorsInfoLog("RAPL units power: 0x%x energy: 0x%x time: 0x%x", power_units, energy_units, time_units);
            
            if (energy_units && (energyUnits = 1.0f / (float)(1 << energy_units))) {
                if (!addSensor(KEY_CPU_PACKAGE_TOTAL_POWER, TYPE_SP78, TYPE_SPXX_SIZE, kCPUSensorsTotalPowerSensor, 0))
                    HWSensorsWarningLog("failed to add CPU package total power sensor");
                
                if (!addSensor(KEY_CPU_PACKAGE_CORE_POWER, TYPE_SP78, TYPE_SPXX_SIZE, kCPUSensorsCoresPowerSensor, 1))
                        HWSensorsWarningLog("failed to add CPU package cores power sensor");
                
                // Uncore sensor is only available on CPUs with uncore device (built-in GPU)
                if (cpuid_info()->cpuid_model != CPUID_MODEL_JAKETOWN && cpuid_info()->cpuid_model != CPUID_MODEL_IVYBRIDGE_EP) {
                    if (!addSensor(KEY_CPU_PACKAGE_GFX_POWER, TYPE_SP78, TYPE_SPXX_SIZE, kCPUSensorsUncorePowerSensor, 2))
                        HWSensorsWarningLog("failed to add CPU package uncore power sensor");
                }
                
                switch (cpuid_info()->cpuid_cpufamily) {
                    case CPUFAMILY_INTEL_HASWELL:
                    case CPUFAMILY_INTEL_HASWELL_ULT:
                        // TODO: check DRAM availability for other platforms
                        if (cpuid_info()->cpuid_cpufamily != CPUFAMILY_INTEL_SANDYBRIDGE) {
                            if (!addSensor(KEY_CPU_PACKAGE_DRAM_POWER, TYPE_SP78, TYPE_SPXX_SIZE, kCPUSensorsDramPowerSensor, 3))
                                HWSensorsWarningLog("failed to add CPU package DRAM power sensor");
                        }
                }
            }
            break;
        }
            
    }
    
    disableExclusiveAccessMode();
    
    // Register service
    registerService();
    
    // start timer
    timerEventsMomentum = 0;
    timerEventSource->setTimeoutMS(500);
    
    return true;
}

void CPUSensors::stop(IOService *provider)
{
    timerEventSource->cancelTimeout();
    workloop->removeEventSource(timerEventSource);
    
    super::stop(provider);
}

void CPUSensors::free()
{
    super::free();
}