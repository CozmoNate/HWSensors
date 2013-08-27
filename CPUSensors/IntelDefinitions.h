//
//  IntelDefinitions.h
//  HWSensors
//
//  Created by Kozlek on 27/08/13.
//
//

/*
 Copyright (c) 2009-2012, Intel Corporation
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// written by Roman Dementiev
//            Austen Ott


#ifndef HWSensors_IntelDefinitions_h
#define HWSensors_IntelDefinitions_h

#define MSR_IA32_CR_PERF_GLOBAL_CTRL        0x38F
#define MSR_IA32_CR_FIXED_CTR_CTRL          0x38D

#define MSR_CPU_CLK_UNHALTED_THREAD_ADDR    0x30A
#define MSR_CPU_CLK_UNHALTED_REF_ADDR       0x30B

struct FixedEventControlRegister
{
    union
    {
        struct
        {
            // CTR0
            UInt64 os0 : 1;
            UInt64 usr0 : 1;
            UInt64 any_thread0 : 1;
            UInt64 enable_pmi0 : 1;
            // CTR1
            UInt64 os1 : 1;
            UInt64 usr1 : 1;
            UInt64 any_thread1 : 1;
            UInt64 enable_pmi1 : 1;
            // CTR2
            UInt64 os2 : 1;
            UInt64 usr2 : 1;
            UInt64 any_thread2 : 1;
            UInt64 enable_pmi2 : 1;
            
            UInt64 reserved1 : 52;
        } fields;
        UInt64 value;
    };
};

inline void init_cpu_turbo_counters(void *magic)
{
    //UInt32 number = get_cpu_number();
    
    FixedEventControlRegister ctrl_reg;
    ctrl_reg.value = rdmsr64(MSR_IA32_CR_FIXED_CTR_CTRL);
    
    
    ctrl_reg.fields.os0 = 1;
    ctrl_reg.fields.usr0 = 1;
    ctrl_reg.fields.any_thread0 = 0;
    ctrl_reg.fields.enable_pmi0 = 0;
    
    ctrl_reg.fields.os1 = 1;
    ctrl_reg.fields.usr1 = 1;
    ctrl_reg.fields.any_thread1 = 0;
    ctrl_reg.fields.enable_pmi1 = 0;
    
    ctrl_reg.fields.os2 = 1;
    ctrl_reg.fields.usr2 = 1;
    ctrl_reg.fields.any_thread2 = 0;
    ctrl_reg.fields.enable_pmi2 = 0;
    
    wrmsr64(MSR_IA32_CR_FIXED_CTR_CTRL, ctrl_reg.value);
    
    // start counting
    UInt64 value = (1ULL << 0) + (1ULL << 1) + (1ULL << 2) + (1ULL << 3) + (1ULL << 32) + (1ULL << 33) + (1ULL << 34);
    wrmsr64(MSR_IA32_CR_PERF_GLOBAL_CTRL, value);
}

#endif
