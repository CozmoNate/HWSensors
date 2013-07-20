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

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IORegistryEntry.h>

#define kCPUSensorsPackageTemperatureSensor   1000

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(CPUSensors, FakeSMCPlugin)

inline UInt8 get_hex_index(char c)
{       
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
};

inline UInt32 get_cpu_number()
{
    return cpu_number() % cpuid_info()->core_count;
}

inline void read_cpu_thermal(void* cpu_index)
{
    UInt8 * cpn = (UInt8 *)cpu_index;
    
	*cpn = get_cpu_number();
    
	if(*cpn < kCPUSensorsMaxCpus) {
		UInt64 msr = rdmsr64(MSR_IA32_THERM_STS);
		if (msr & 0x80000000) cpu_thermal[*cpn] = (msr >> 16) & 0x7F;
	}
};

inline void read_cpu_package_thermal(void* magic)
{
    UInt64 msr = rdmsr64(MSR_IA32_PACKAGE_THERM_STATUS);
    cpu_package_thermal = (msr >> 16) & 0x7F;
};

inline void read_cpu_performance(void* cpu_index)
{
    UInt8 *cpn = (UInt8*)cpu_index;
    
	*cpn = get_cpu_number();
    
	if(*cpn < kCPUSensorsMaxCpus) {
		UInt64 msr = rdmsr64(MSR_IA32_PERF_STS);
        cpu_performance[*cpn] = msr & 0xFFFF;
	}
};

void CPUSensors::readTjmaxFromMSR()
{
	for (uint32_t i = 0; i < cpuid_info()->core_count; i++) {
		tjmax[i] = (rdmsr64(MSR_IA32_TEMP_TARGET) >> 16) & 0xFF;
	}
}

IOReturn CPUSensors::loopTimerEvent(void)
{
    UInt8 index;
    
    if (thermCounter++ < 4) {
        for (UInt8 i = 0; i < cpuid_info()->core_count; i++) {
            mp_rendezvous_no_intrs(read_cpu_thermal, &index);
            IOSleep(1); // Yield?
        }
        
        if (cpuid_info()->cpuid_thermal_sensor) {
            mp_rendezvous_no_intrs(read_cpu_package_thermal, &index);
            IOSleep(1); // Yield?
        }
    }
    
    if (perfCounter++ < 4) {
        switch (cpuid_info()->cpuid_cpufamily) {
            case CPUFAMILY_INTEL_NEHALEM:
            case CPUFAMILY_INTEL_WESTMERE:
            case CPUFAMILY_INTEL_SANDYBRIDGE:
            case CPUFAMILY_INTEL_IVYBRIDGE:
            case CPUFAMILY_INTEL_HASWELL:
            case CPUFAMILY_INTEL_HASWELL_ULT:
                mp_rendezvous_no_intrs(read_cpu_performance, &index);
                IOSleep(1); // Yield?
                cpu_performance[0] = cpu_performance[index];
                break;
                
            default:
                for (UInt8 i = 0; i < cpuid_info()->core_count; i++) {
                    mp_rendezvous_no_intrs(read_cpu_performance, &index);
                    IOSleep(1); // Yield?
                }
                break;
        }
    }
    
    timersource->setTimeoutMS(1000);
    
    return kIOReturnSuccess;
}

float CPUSensors::calculateMultiplier(UInt8 cpu_index)
{
    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_NEHALEM:
        case CPUFAMILY_INTEL_WESTMERE:
            return cpu_performance[0];
            
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_HASWELL_ULT:
            return cpu_performance[0] >> 8;

        default: {
            UInt8 fid = cpu_performance[cpu_index] >> 8;
            return (float)((fid & 0x1f)) * (fid & 0x80 ? 0.5 : 1.0) + 0.5f * (float)((fid >> 6) & 1);
        }
    }
    
    return 0;
}

float CPUSensors::getSensorValue(FakeSMCSensor *sensor)
{
    switch (sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            if (sensor->getIndex() < cpuid_info()->core_count) {
                thermCounter = 0;
                return tjmax[sensor->getIndex()] - cpu_thermal[sensor->getIndex()];
            }	
            break;
            
        case kFakeSMCMultiplierSensor: 
            if (sensor->getIndex() < cpuid_info()->core_count) {
                perfCounter = 0;
                return calculateMultiplier(sensor->getIndex());
            }
            break;
            
        case kFakeSMCFrequencySensor:
            if (sensor->getIndex() < cpuid_info()->core_count) {
                perfCounter = 0;
                return calculateMultiplier(sensor->getIndex()) * (float)busClock;
            }
            break;
            
        case kCPUSensorsPackageTemperatureSensor:
            return tjmax[0] - cpu_package_thermal;
    }
    
    return 0;
}

IOService *CPUSensors::probe(IOService *provider, SInt32 *score)
{
    if (super::probe(provider, score) != this) 
        return 0;
    
    if (!(workloop = getWorkLoop())) 
		return 0;
	
	if (!(timersource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &CPUSensors::loopTimerEvent)))) 
		return 0;
	
	if (kIOReturnSuccess != workloop->addEventSource(timersource))
		return 0;
    
    return this;
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
            if (string->getLength() > 0)
                platform = OSString::withString(string);
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
                        if (!platform) platform = OSString::withCString("M70\0\0\0\0\0");
                        break;
                            
                    case CPUID_MODEL_YONAH:
                        if (!platform) platform = OSString::withCString("K22\0\0\0\0\0");
                        tjmax[0] = 85;
                        break;
                        
                    case CPUID_MODEL_MEROM: // Intel Core (65nm)
                        if (!platform) platform = OSString::withCString("M75\0\0\0\0\0");
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
                        if (!platform) platform = OSString::withCString("M82\0\0\0\0\0");
                        if (rdmsr64(0x17) & (1<<28))
                            tjmax[0] = 105;
                        else
                            tjmax[0] = 100; 
                        break;
                        
                    case CPUID_MODEL_ATOM: // Intel Atom (45nm)
                        if (!platform) platform = OSString::withCString("T9\0\0\0\0\0");
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
                        if (!platform) platform = OSString::withCString("k74\0\0\0\0\0");
                        readTjmaxFromMSR();
                        break;
                        
                    case CPUID_MODEL_SANDYBRIDGE:
                    case CPUID_MODEL_JAKETOWN:
                        if (!platform) platform = OSString::withCString("k62\0\0\0\0\0");
                        readTjmaxFromMSR();
                        break;
                        
                    case CPUID_MODEL_IVYBRIDGE:
                        if (!platform) platform = OSString::withCString("d8\0\0\0\0\0\0");
                        readTjmaxFromMSR();
                        break;
                    
                    case CPUID_MODEL_HASWELL_DT:
                    case CPUID_MODEL_HASWELL_MB:
                    case CPUID_MODEL_HASWELL_ULT:
                    case CPUID_MODEL_HASWELL_ULX:
                        if (!platform) platform = OSString::withCString("j43\0\0\0\0\0"); // TODO: got from macbookair6,2 need to check for other platforms
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
	
    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_NEHALEM:
        case CPUFAMILY_INTEL_WESTMERE:
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_HASWELL_ULT:
            break;
            
        default:
            for (uint32_t i = 1; i < cpuid_info()->core_count; i++) tjmax[i] = tjmax[0];
            break;
    }
    
    busClock = 0;
    
    if (IORegistryEntry *regEntry = fromPath("/efi/platform", gIODTPlane))
        if (OSData *data = OSDynamicCast(OSData, regEntry->getProperty("FSBFrequency")))
            busClock = *((UInt64*) data->getBytesNoCopy()) / 1e6;
    
    if (busClock == 0)
        busClock = (gPEClockFrequencyInfo.bus_frequency_max_hz >> 2) / 1e6;
    
    HWSensorsInfoLog("CPU family 0x%x, model 0x%x, stepping 0x%x, cores %d, threads %d, TJmax %d", cpuid_info()->cpuid_family, cpuid_info()->cpuid_model, cpuid_info()->cpuid_stepping, cpuid_info()->core_count, cpuid_info()->thread_count, tjmax[0]);
    
    if (platform) {
        HWSensorsInfoLog("setting platform to %s", platform->getCStringNoCopy());
        
        if (!isKeyExists("RPlt") && !setKeyValue("RPlt", TYPE_CH8, platform->getLength(), (void*)platform->getCStringNoCopy()))
            HWSensorsWarningLog("failed to set platform key RPlt");
        
        if (!isKeyExists("RBr") && !setKeyValue("RBr", TYPE_CH8, platform->getLength(), (void*)platform->getCStringNoCopy()))
                HWSensorsWarningLog("failed to set platform key RBr");
    }
	
	for (uint32_t i = 0; i < cpuid_info()->core_count; i++) {
        
        if (i >= kCPUSensorsMaxCpus) 
            break;
        
        char key[5];
		
		snprintf(key, 5, KEY_FORMAT_CPU_DIODE_TEMPERATURE, i);
        
        if (!addSensor(key, TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, i))
			HWSensorsWarningLog("failed to add temperature sensor");
		
        switch (cpuid_info()->cpuid_cpufamily) {
            case CPUFAMILY_INTEL_NEHALEM:
            case CPUFAMILY_INTEL_WESTMERE:
            case CPUFAMILY_INTEL_SANDYBRIDGE:
            case CPUFAMILY_INTEL_IVYBRIDGE:
            case CPUFAMILY_INTEL_HASWELL:
            case CPUFAMILY_INTEL_HASWELL_ULT:
                break;
                
            default:
                snprintf(key, 5, KEY_FAKESMC_FORMAT_CPU_MULTIPLIER, i);
                
                if (!addSensor(key, TYPE_FP88, TYPE_FPXX_SIZE, kFakeSMCMultiplierSensor, i))
                    HWSensorsWarningLog("failed to add multiplier sensor");
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_CPU_FREQUENCY, i);
                
                if (!addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, i))
                    HWSensorsWarningLog("failed to add frequency sensor");
                
                break;
        }
	}
    
    if (cpuid_info()->cpuid_thermal_sensor) {
        if (!addSensor(KEY_CPU_PACKAGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kCPUSensorsPackageTemperatureSensor, 0))
            HWSensorsWarningLog("failed to add cpu package temperature sensor");
    }
    
    switch (cpuid_info()->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_NEHALEM:
        case CPUFAMILY_INTEL_WESTMERE:
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_HASWELL_ULT:
            if (!addSensor(KEY_FAKESMC_CPU_PACKAGE_MULTIPLIER, TYPE_FP88, TYPE_FPXX_SIZE, kFakeSMCMultiplierSensor, 0))
                HWSensorsWarningLog("failed to add package multiplier sensor");
            if (!addSensor(KEY_FAKESMC_CPU_PACKAGE_FREQUENCY, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, 0))
                HWSensorsWarningLog("failed to add package frequency sensor");
            break;
            
        default:
            break;
    }
    
    loopTimerEvent();
    
    registerService();
    
    return true;
}

void CPUSensors::stop(IOService *provider)
{
    timersource->disable();
    workloop->removeEventSource(timersource);
    
    super::stop(provider);
}

void CPUSensors::free(void)
{
    timersource->release();
    
    super::free();
}
