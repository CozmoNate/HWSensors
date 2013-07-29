/*
 *  HWSensors.h
 *  CPUSensorsPlugin
 *
 *  Based on code by mercurysquad, superhai (C) 2008
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
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

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

#include <IOKit/IOTimerEventSource.h>

#include "FakeSMCPlugin.h"

#include "cpuid.h"

#define MSR_IA32_THERM_STS              0x019C
#define MSR_IA32_TEMP_TARGET            0x01A2
#define MSR_IA32_PERF_STATUS            0x0198

#define MSR_RAPL_POWER_UNIT             0x606
#define MSR_PKG_ENERY_STATUS            0x611
#define MSR_DRAM_ENERGY_STATUS          0x619
#define MSR_PP0_ENERY_STATUS            0x639
#define MSR_PP1_ENERY_STATUS            0x641

#define MSR_IA32_APERF                  0x0E8
#define MSR_IA32_MPERF                  0x0E7

#define MSR_PERF_FIXED_CTR_CTRL         0x38d
#define MSR_PERF_GLOBAL_CTRL            0x38f

#define kCPUSensorsMaxCpus              64

extern "C" int cpu_number(void);
extern "C" void mp_rendezvous_no_intrs(void (*action_func)(void *), void * arg);

static UInt16 cpu_energy_msrs[] =
{
    MSR_PKG_ENERY_STATUS,
    MSR_PP0_ENERY_STATUS,
    MSR_PP1_ENERY_STATUS,
    MSR_DRAM_ENERGY_STATUS
};

class CPUSensors : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(CPUSensors)    
    
private:
    UInt8                   tjmax[kCPUSensorsMaxCpus];
    OSData*                 platform;
    UInt64                  busClock;
    float                   multiplier[kCPUSensorsMaxCpus];
    double                  lastEnergyTime[4];
    UInt64                  lastEnergyValue[4];
    float                   energyUnitValue;
    UInt8                   baseMultiplier;
    
	void                    readTjmaxFromMSR();
    float                   readMultiplier(UInt8 cpu_index);
    float                   readFrequency(UInt8 cpu_index);
    
protected:
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual bool			start(IOService *provider);
};