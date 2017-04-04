/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */

#ifndef M_CPUID_H
#define M_CPUID_H

#include <mach/machine.h>
#include <machine/machine_routines.h>
#include <pexpert/pexpert.h>
#include <i386/proc_reg.h>
#include <string.h>

#define CPUID_DEBUG   0


#define	CPUID_VID_INTEL		"GenuineIntel"
#define	CPUID_VID_AMD		"AuthenticAMD"

#define CPUID_STRING_UNKNOWN    "Unknown CPU Type"

//#define MSR_CORE_THREAD_COUNT 0x035

#define _Bit(n)			(1ULL << n)
#define _HBit(n)		(1ULL << ((n)+32))

#define min(a,b) ((a) < (b) ? (a) : (b))
#define quad(hi,lo)	(((uint64_t)(hi)) << 32 | (lo))

/* Only for 32bit values */
#define bit32(n)            (1U << (n))
#define bitmask32(h,l)		((bit32(h)|(bit32(h)-1)) & ~(bit32(l)-1))
#define bitfield32(x,h,l)	((((x) & bitmask32(h,l)) >> l))

/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1:
 */
#define	CPUID_FEATURE_FPU     _Bit(0)	/* Floating point unit on-chip */
#define	CPUID_FEATURE_VME     _Bit(1)	/* Virtual Mode Extension */
#define	CPUID_FEATURE_DE      _Bit(2)	/* Debugging Extension */
#define	CPUID_FEATURE_PSE     _Bit(3)	/* Page Size Extension */
#define	CPUID_FEATURE_TSC     _Bit(4)	/* Time Stamp Counter */
#define	CPUID_FEATURE_MSR     _Bit(5)	/* Model Specific Registers */
#define CPUID_FEATURE_PAE     _Bit(6)	/* Physical Address Extension */
#define	CPUID_FEATURE_MCE     _Bit(7)	/* Machine Check Exception */
#define	CPUID_FEATURE_CX8     _Bit(8)	/* CMPXCHG8B */
#define	CPUID_FEATURE_APIC    _Bit(9)	/* On-chip APIC */
#define CPUID_FEATURE_SEP     _Bit(11)	/* Fast System Call */
#define	CPUID_FEATURE_MTRR    _Bit(12)	/* Memory Type Range Register */
#define	CPUID_FEATURE_PGE     _Bit(13)	/* Page Global Enable */
#define	CPUID_FEATURE_MCA     _Bit(14)	/* Machine Check Architecture */
#define	CPUID_FEATURE_CMOV    _Bit(15)	/* Conditional Move Instruction */
#define CPUID_FEATURE_PAT     _Bit(16)	/* Page Attribute Table */
#define CPUID_FEATURE_PSE36   _Bit(17)	/* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN     _Bit(18)	/* Processor Serial Number */
#define CPUID_FEATURE_CLFSH   _Bit(19)	/* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS      _Bit(21)	/* Debug Store */
#define CPUID_FEATURE_ACPI    _Bit(22)	/* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX     _Bit(23)	/* MMX supported */
#define CPUID_FEATURE_FXSR    _Bit(24)	/* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE     _Bit(25)	/* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2    _Bit(26)	/* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS      _Bit(27)	/* Self-Snoop */
#define CPUID_FEATURE_HTT     _Bit(28)	/* Hyper-Threading Technology */
#define CPUID_FEATURE_TM      _Bit(29)	/* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE     _Bit(31)	/* Pend Break Enable */

#define CPUID_FEATURE_SSE3    _HBit(0)	/* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_PCLMULQDQ _HBit(1) /* PCLMULQDQ Instruction */

#define CPUID_FEATURE_MONITOR _HBit(3)	/* Monitor/mwait */
#define CPUID_FEATURE_DSCPL   _HBit(4)	/* Debug Store CPL */
#define CPUID_FEATURE_VMX     _HBit(5)	/* VMX */
#define CPUID_FEATURE_SMX     _HBit(6)	/* SMX */
#define CPUID_FEATURE_EST     _HBit(7)	/* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2     _HBit(8)	/* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3   _HBit(9)	/* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID     _HBit(10)	/* L1 Context ID */

#define CPUID_FEATURE_CX16    _HBit(13)	/* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR    _HBit(14)	/* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM    _HBit(15)	/* Perf/Debug Capability MSR */

#define CPUID_FEATURE_DCA     _HBit(18)	/* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1  _HBit(19)	/* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2  _HBit(20)	/* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_xAPIC   _HBit(21)	/* Extended APIC Mode */
#define CPUID_FEATURE_POPCNT  _HBit(23)	/* POPCNT instruction */
#define CPUID_FEATURE_AES     _HBit(25)	/* AES instructions */
#define CPUID_FEATURE_VMM     _HBit(31)	/* VMM (Hypervisor) present */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001:
 */
#define CPUID_EXTFEATURE_SYSCALL   _Bit(11)	/* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD		   _Bit(20)	/* eXecute Disable */
#define CPUID_EXTFEATURE_1GBPAGE   _Bit(26)     /* 1G-Byte Page support */
#define CPUID_EXTFEATURE_RDTSCP	   _Bit(27)	/* RDTSCP */
#define CPUID_EXTFEATURE_EM64T	   _Bit(29)	/* Extended Mem 64 Technology */

//#define CPUID_EXTFEATURE_LAHF	   _HBit(20)	/* LAFH/SAHF instructions */
// New definition with Snow kernel
#define CPUID_EXTFEATURE_LAHF	   _HBit(0)	/* LAHF/SAHF instructions */
/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007:
 */
#define CPUID_EXTFEATURE_TSCI      _Bit(8)	/* TSC Invariant */

#define	CPUID_CACHE_SIZE	16	/* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION	_Bit(0)	/* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK	_Bit(1)	/* interrupts are break events	   */

#define CPUID_MODEL_PENTIUM_M		0x0D
#define CPUID_MODEL_YONAH			0x0E
#define CPUID_MODEL_MEROM			0x0F
#define CPUID_MODEL_PENRYN			0x17
#define CPUID_MODEL_NEHALEM			0x1A
#define CPUID_MODEL_ATOM			0x1C
#define CPUID_MODEL_FIELDS			0x1E	/* Lynnfield, Clarksfield, Jasper */
#define CPUID_MODEL_DALES			0x1F	/* Havendale, Auburndale */
#define CPUID_MODEL_DALES_32NM		0x25	/* Clarkdale, Arrandale */
#define CPUID_MODEL_SANDYBRIDGE		0x2A
#define CPUID_MODEL_WESTMERE		0x2C	/* Gulftown, Westmere-EP, Westmere-WS */
#define CPUID_MODEL_JAKETOWN        0x2D
#define CPUID_MODEL_NEHALEM_EX		0x2E
#define CPUID_MODEL_WESTMERE_EX		0x2F
#define CPUID_MODEL_IVYBRIDGE       0x3A
#define CPUID_MODEL_HASWELL_DT      0x3C
#define CPUID_MODEL_IVYBRIDGE_EP    0x3E
#define CPUID_MODEL_HASWELL_MB      0x3F    /* Haswell MB */
//#define CPUID_MODEL_HASWELL_H        0x??    // Haswell H
#define CPUID_MODEL_HASWELL_ULT     0x45    /* Haswell ULT */
#define CPUID_MODEL_HASWELL_ULX     0x46    /* Haswell ULX */
#define CPUID_MODEL_BROADWELL_DT    0x47
#define CPUID_MODEL_BROADWELL_ULV   0x3D
#define CPUID_MODEL_BROADWELL_MB    0x4f
#define CPUID_MODEL_SKYLAKE_LT      0x4E
#define CPUID_MODEL_SKYLAKE_DT		0x5E
#define CPUID_MODEL_KABYLAKE_U		0x8E
#define CPUID_MODEL_KABYLAKE_S		0x9E

#ifndef CPUFAMILY_INTEL_SANDYBRIDGE
#define CPUFAMILY_INTEL_SANDYBRIDGE 0x5490b78c // From 10.7
#endif

#ifndef CPUFAMILY_INTEL_IVYBRIDGE
#define CPUFAMILY_INTEL_IVYBRIDGE   0x1f65e835 // From 10.8
#endif

#ifndef CPUFAMILY_INTEL_HASWELL
#define CPUFAMILY_INTEL_HASWELL     0x10B282DC // From 10.9
#endif

#ifndef CPUFAMILY_INTEL_BROADWELL
#define CPUFAMILY_INTEL_BROADWELL	0x582ed09c // From 10.10
#endif

#ifndef CPUFAMILY_INTEL_SKYLAKE
#define CPUFAMILY_INTEL_SKYLAKE     0x37FC219F // From 10.11
#endif

#ifndef CPUFAMILY_INTEL_KABYLAKE
#define CPUFAMILY_INTEL_KABYLAKE    0xFFFFFFFF // Unknown
#endif


//#define CPUFAMILY_INTEL_HASWELL_ULT 0x10B282DC //

//#include <stdint.h>

typedef enum { eax, ebx, ecx, edx } cpuid_register_t;
static inline void
cpuid(uint32_t *data)
{
    asm("cpuid"
        : "=a" (data[eax]),
        "=b" (data[ebx]),
        "=c" (data[ecx]),
        "=d" (data[edx])
        : "a"  (data[eax]),
        "b"  (data[ebx]),
        "c"  (data[ecx]),
        "d"  (data[edx]));
}

static inline void
do_cpuid(uint32_t selector, uint32_t *data)
{
    asm("cpuid"
        : "=a" (data[0]),
        "=b" (data[1]),
        "=c" (data[2]),
        "=d" (data[3])
        : "a"(selector),
        "b" (0),
        "c" (0),
        "d" (0));
}

/*
 * Cache ID descriptor structure, used to parse CPUID leaf 2.
 * Note: not used in kernel.
 */
typedef enum { Lnone, L1I, L1D, L2U, L3U, LCACHE_MAX } cache_type_t ;
typedef struct {
    unsigned char	value;          /* Descriptor value */
    cache_type_t 	type;           /* Cache type */
    unsigned int 	size;           /* Cache size */
    unsigned int 	linesize;       /* Cache line size */
#ifdef KERNEL
    const char	*description;   /* Cache description */
#endif /* KERNEL */
} cpuid_cache_desc_t;

#ifdef KERNEL
#define CACHE_DESC(value,type,size,linesize,text) \
{ value, type, size, linesize, text }
#else
#define CACHE_DESC(value,type,size,linesize,text) \
{ value, type, size, linesize }
#endif /* KERNEL */


/* Monitor/mwait Leaf: */
typedef struct {
    uint32_t	linesize_min;
    uint32_t	linesize_max;
    uint32_t	extensions;
    uint32_t	sub_Cstates;
} cpuid_mwait_leaf_t;

/* Thermal and Power Management Leaf: */
typedef struct {
    boolean_t	sensor;
    boolean_t	dynamic_acceleration;
    boolean_t	invariant_APIC_timer;
    boolean_t	core_power_limits;
    boolean_t	fine_grain_clock_mod;
    boolean_t	package_thermal_intr;
    uint32_t	thresholds;
    boolean_t	ACNT_MCNT;
    boolean_t	hardware_feedback;
    boolean_t	energy_policy;
} cpuid_thermal_leaf_t;


/* XSAVE Feature Leaf: */
typedef struct {
    uint32_t	extended_state[4];	/* eax .. edx */
} cpuid_xsave_leaf_t;


/* Architectural Performance Monitoring Leaf: */
typedef struct {
    uint8_t		version;
    uint8_t		number;
    uint8_t		width;
    uint8_t		events_number;
    uint32_t	events;
    uint8_t		fixed_number;
    uint8_t		fixed_width;
} cpuid_arch_perf_leaf_t;

/* Physical CPU info - this is exported out of the kernel (kexts), so be wary of changes */
typedef struct {
    char		cpuid_vendor[16];
    char		cpuid_brand_string[48];
    const char	*cpuid_model_string;
    
    cpu_type_t	cpuid_type;	/* this is *not* a cpu_type_t in our <mach/machine.h> */
    uint8_t		cpuid_family;
    uint8_t		cpuid_model;
    uint8_t		cpuid_extmodel;
    uint8_t		cpuid_extfamily;
    uint8_t		cpuid_stepping;
    uint64_t	cpuid_features;
    uint64_t	cpuid_extfeatures;
    uint32_t	cpuid_signature;
    uint8_t   	cpuid_brand;
    uint8_t		cpuid_processor_flag;
    
    uint32_t	cache_size[LCACHE_MAX];
    uint32_t	cache_linesize;
    
    uint8_t		cache_info[64];    /* list of cache descriptors */
    
    uint32_t	cpuid_cores_per_package;
    uint32_t	cpuid_logical_per_package;
    uint32_t	cache_sharing[LCACHE_MAX];
    uint32_t	cache_partitions[LCACHE_MAX];
    
    cpu_type_t	cpuid_cpu_type;			/* <mach/machine.h> */
    cpu_subtype_t	cpuid_cpu_subtype;		/* <mach/machine.h> */
    
    /* Per-vendor info */
    cpuid_mwait_leaf_t	cpuid_mwait_leaf;
#define cpuid_mwait_linesize_max	cpuid_mwait_leaf.linesize_max
#define cpuid_mwait_linesize_min	cpuid_mwait_leaf.linesize_min
#define cpuid_mwait_extensions		cpuid_mwait_leaf.extensions
#define cpuid_mwait_sub_Cstates		cpuid_mwait_leaf.sub_Cstates
    cpuid_thermal_leaf_t	cpuid_thermal_leaf;
    cpuid_arch_perf_leaf_t	cpuid_arch_perf_leaf;
    cpuid_xsave_leaf_t	cpuid_xsave_leaf;
    
    /* Cache details: */
    uint32_t	cpuid_cache_linesize;
    uint32_t	cpuid_cache_L2_associativity;
    uint32_t	cpuid_cache_size;
    
    /* Virtual and physical address aize: */
    uint32_t	cpuid_address_bits_physical;
    uint32_t	cpuid_address_bits_virtual;
    
    uint32_t	cpuid_microcode_version;
    
    /* Numbers of tlbs per processor [i|d, small|large, level0|level1] */
    uint32_t	cpuid_tlb[2][2][2];
#define	TLB_INST	0
#define	TLB_DATA	1
#define	TLB_SMALL	0
#define	TLB_LARGE	1
    uint32_t	cpuid_stlb;
    
    uint32_t	core_count;
    uint32_t	thread_count;
    
    /* Max leaf ids available from CPUID */
    uint32_t	cpuid_max_basic;
    uint32_t	cpuid_max_ext;
    
    /* Family-specific info links */
    uint32_t		cpuid_cpufamily;
    cpuid_mwait_leaf_t	*cpuid_mwait_leafp;
    cpuid_thermal_leaf_t	*cpuid_thermal_leafp;
    cpuid_arch_perf_leaf_t	*cpuid_arch_perf_leafp;
    cpuid_xsave_leaf_t	*cpuid_xsave_leafp;
    uint32_t		cpuid_leaf7_features;
} i386_cpu_info_t;

static i386_cpu_info_t cpuid_cpu_info;

inline static i386_cpu_info_t	*cpuid_info(void)
{
    return 	&cpuid_cpu_info;
}

static	boolean_t	cpuid_dbg
#if CPUID_DEBUG
= TRUE;
#else
= FALSE;
#endif

#define DBG(x...)			\
do {				\
if (cpuid_dbg)		\
IOLog(x);	\
} while (0)			\

static void cpuid_fn(uint32_t selector, uint32_t *result)
{
    do_cpuid(selector, result);
    DBG("cpuid_fn(0x%08x) eax:0x%08x ebx:0x%08x ecx:0x%08x edx:0x%08x\n",
        selector, result[0], result[1], result[2], result[3]);
}

static void
cpuid_set_generic_info(i386_cpu_info_t *info_p)
{
    uint32_t	reg[4];
    char            str[128], *p;
    
    DBG("cpuid_set_generic_info(%p)\n", info_p);
    
    /* do cpuid 0 to get vendor */
    cpuid_fn(0, reg);
    info_p->cpuid_max_basic = reg[eax];
    bcopy((char *)&reg[ebx], &info_p->cpuid_vendor[0], 4); /* ug */
    bcopy((char *)&reg[ecx], &info_p->cpuid_vendor[8], 4);
    bcopy((char *)&reg[edx], &info_p->cpuid_vendor[4], 4);
    info_p->cpuid_vendor[12] = 0;
    
    /* get extended cpuid results */
    cpuid_fn(0x80000000, reg);
    info_p->cpuid_max_ext = reg[eax];
    
    /* check to see if we can get brand string */
    if (info_p->cpuid_max_ext >= 0x80000004) {
        /*
         * The brand string 48 bytes (max), guaranteed to
         * be NUL terminated.
         */
        cpuid_fn(0x80000002, reg);
        bcopy((char *)reg, &str[0], 16);
        cpuid_fn(0x80000003, reg);
        bcopy((char *)reg, &str[16], 16);
        cpuid_fn(0x80000004, reg);
        bcopy((char *)reg, &str[32], 16);
        for (p = str; *p != '\0'; p++) {
            if (*p != ' ') break;
        }
        strlcpy(info_p->cpuid_brand_string,
                p, sizeof(info_p->cpuid_brand_string));
        
        if (!strncmp(info_p->cpuid_brand_string, CPUID_STRING_UNKNOWN,
                     min(sizeof(info_p->cpuid_brand_string),
                         strlen(CPUID_STRING_UNKNOWN) + 1))) {
                         /*
                          * This string means we have a firmware-programmable brand string,
                          * and the firmware couldn't figure out what sort of CPU we have.
                          */
                         info_p->cpuid_brand_string[0] = '\0';
                     }
    }
    
    /* Get cache and addressing info. */
    if (info_p->cpuid_max_ext >= 0x80000006) {
        uint32_t assoc;
        cpuid_fn(0x80000006, reg);
        info_p->cpuid_cache_linesize   = bitfield32(reg[ecx], 7, 0);
        assoc = bitfield32(reg[ecx],15,12);
        /*
         * L2 associativity is encoded, though in an insufficiently
         * descriptive fashion, e.g. 24-way is mapped to 16-way.
         * Represent a fully associative cache as 0xFFFF.
         * Overwritten by associativity as determined via CPUID.4
         * if available.
         */
        if (assoc == 6)
            assoc = 8;
        else if (assoc == 8)
            assoc = 16;
        else if (assoc == 0xF)
            assoc = 0xFFFF;
        info_p->cpuid_cache_L2_associativity = assoc;
        info_p->cpuid_cache_size       = bitfield32(reg[ecx],31,16);
        cpuid_fn(0x80000008, reg);
        info_p->cpuid_address_bits_physical =
        bitfield32(reg[eax], 7, 0);
        info_p->cpuid_address_bits_virtual =
        bitfield32(reg[eax],15, 8);
    }
    
    /*
     * Get processor signature and decode
     * and bracket this with the approved procedure for reading the
     * the microcode version number a.k.a. signature a.k.a. BIOS ID
     */
    wrmsr64(MSR_IA32_BIOS_SIGN_ID, 0);
    cpuid_fn(1, reg);
    info_p->cpuid_microcode_version =
    (uint32_t) (rdmsr64(MSR_IA32_BIOS_SIGN_ID) >> 32);
    info_p->cpuid_signature = reg[eax];
    info_p->cpuid_stepping  = bitfield32(reg[eax],  3,  0);
    info_p->cpuid_model     = bitfield32(reg[eax],  7,  4);
    info_p->cpuid_family    = bitfield32(reg[eax], 11,  8);
    info_p->cpuid_type      = bitfield32(reg[eax], 13, 12);
    info_p->cpuid_extmodel  = bitfield32(reg[eax], 19, 16);
    info_p->cpuid_extfamily = bitfield32(reg[eax], 27, 20);
    info_p->cpuid_brand     = bitfield32(reg[ebx],  7,  0);
    info_p->cpuid_features  = quad(reg[ecx], reg[edx]);
    
    /* Get "processor flag"; necessary for microcode update matching */
    info_p->cpuid_processor_flag = (rdmsr64(MSR_IA32_PLATFORM_ID)>> 50) & 0x7;
    
    /* Fold extensions into family/model */
    if (info_p->cpuid_family == 0x0f)
        info_p->cpuid_family += info_p->cpuid_extfamily;
    if (info_p->cpuid_family == 0x0f || info_p->cpuid_family == 0x06)
        info_p->cpuid_model += (info_p->cpuid_extmodel << 4);
    
    if (info_p->cpuid_features & CPUID_FEATURE_HTT)
        info_p->cpuid_logical_per_package =
        bitfield32(reg[ebx], 23, 16);
    else
        info_p->cpuid_logical_per_package = 1;
    
    if (info_p->cpuid_max_ext >= 0x80000001) {
        cpuid_fn(0x80000001, reg);
        info_p->cpuid_extfeatures =
        quad(reg[ecx], reg[edx]);
    }
    
    DBG(" max_basic           : %d\n", info_p->cpuid_max_basic);
    DBG(" max_ext             : 0x%08x\n", info_p->cpuid_max_ext);
    DBG(" vendor              : %s\n", info_p->cpuid_vendor);
    DBG(" brand_string        : %s\n", info_p->cpuid_brand_string);
    DBG(" signature           : 0x%08x\n", info_p->cpuid_signature);
    DBG(" stepping            : %d\n", info_p->cpuid_stepping);
    DBG(" model               : %d\n", info_p->cpuid_model);
    DBG(" family              : %d\n", info_p->cpuid_family);
    DBG(" type                : %d\n", info_p->cpuid_type);
    DBG(" extmodel            : %d\n", info_p->cpuid_extmodel);
    DBG(" extfamily           : %d\n", info_p->cpuid_extfamily);
    DBG(" brand               : %d\n", info_p->cpuid_brand);
    DBG(" features            : 0x%016llx\n", info_p->cpuid_features);
    DBG(" extfeatures         : 0x%016llx\n", info_p->cpuid_extfeatures);
    DBG(" logical_per_package : %d\n", info_p->cpuid_logical_per_package);
    DBG(" microcode_version   : 0x%08x\n", info_p->cpuid_microcode_version);
    
    /* Fold in the Invariant TSC feature bit, if present */
    if (info_p->cpuid_max_ext >= 0x80000007) {
        cpuid_fn(0x80000007, reg);
        info_p->cpuid_extfeatures |=
        reg[edx] & (uint32_t)CPUID_EXTFEATURE_TSCI;
        DBG(" extfeatures         : 0x%016llx\n",
            info_p->cpuid_extfeatures);
    }
    
    if (info_p->cpuid_max_basic >= 0x5) {
        cpuid_mwait_leaf_t	*cmp = &info_p->cpuid_mwait_leaf;
        
        /*
         * Extract the Monitor/Mwait Leaf info:
         */
        cpuid_fn(5, reg);
        cmp->linesize_min = reg[eax];
        cmp->linesize_max = reg[ebx];
        cmp->extensions   = reg[ecx];
        cmp->sub_Cstates  = reg[edx];
        info_p->cpuid_mwait_leafp = cmp;
        
        DBG(" Monitor/Mwait Leaf:\n");
        DBG("  linesize_min : %d\n", cmp->linesize_min);
        DBG("  linesize_max : %d\n", cmp->linesize_max);
        DBG("  extensions   : %d\n", cmp->extensions);
        DBG("  sub_Cstates  : 0x%08x\n", cmp->sub_Cstates);
    }
    
    if (info_p->cpuid_max_basic >= 0x6) {
        cpuid_thermal_leaf_t	*ctp = &info_p->cpuid_thermal_leaf;
        
        /*
         * The thermal and Power Leaf:
         */
        cpuid_fn(6, reg);
        ctp->sensor 		  = bitfield32(reg[eax], 0, 0);
        ctp->dynamic_acceleration = bitfield32(reg[eax], 1, 1);
        ctp->invariant_APIC_timer = bitfield32(reg[eax], 2, 2);
        ctp->core_power_limits    = bitfield32(reg[eax], 4, 4);
        ctp->fine_grain_clock_mod = bitfield32(reg[eax], 5, 5);
        ctp->package_thermal_intr = bitfield32(reg[eax], 6, 6);
        ctp->thresholds		  = bitfield32(reg[ebx], 3, 0);
        ctp->ACNT_MCNT		  = bitfield32(reg[ecx], 0, 0);
        ctp->hardware_feedback	  = bitfield32(reg[ecx], 1, 1);
        ctp->energy_policy	  = bitfield32(reg[ecx], 3, 3);
        info_p->cpuid_thermal_leafp = ctp;
        
        DBG(" Thermal/Power Leaf:\n");
        DBG("  sensor               : %d\n", ctp->sensor);
        DBG("  dynamic_acceleration : %d\n", ctp->dynamic_acceleration);
        DBG("  invariant_APIC_timer : %d\n", ctp->invariant_APIC_timer);
        DBG("  core_power_limits    : %d\n", ctp->core_power_limits);
        DBG("  fine_grain_clock_mod : %d\n", ctp->fine_grain_clock_mod);
        DBG("  package_thermal_intr : %d\n", ctp->package_thermal_intr);
        DBG("  thresholds           : %d\n", ctp->thresholds);
        DBG("  ACNT_MCNT            : %d\n", ctp->ACNT_MCNT);
        DBG("  ACNT2                : %d\n", ctp->hardware_feedback);
        DBG("  energy_policy        : %d\n", ctp->energy_policy);
    }
    
    if (info_p->cpuid_max_basic >= 0xa) {
        cpuid_arch_perf_leaf_t	*capp = &info_p->cpuid_arch_perf_leaf;
        
        /*
         * Architectural Performance Monitoring Leaf:
         */
        cpuid_fn(0xa, reg);
        capp->version	    = bitfield32(reg[eax],  7,  0);
        capp->number	    = bitfield32(reg[eax], 15,  8);
        capp->width	    = bitfield32(reg[eax], 23, 16);
        capp->events_number = bitfield32(reg[eax], 31, 24);
        capp->events	    = reg[ebx];
        capp->fixed_number  = bitfield32(reg[edx],  4,  0);
        capp->fixed_width   = bitfield32(reg[edx], 12,  5);
        info_p->cpuid_arch_perf_leafp = capp;
        
        DBG(" Architectural Performance Monitoring Leaf:\n");
        DBG("  version       : %d\n", capp->version);
        DBG("  number        : %d\n", capp->number);
        DBG("  width         : %d\n", capp->width);
        DBG("  events_number : %d\n", capp->events_number);
        DBG("  events        : %d\n", capp->events);
        DBG("  fixed_number  : %d\n", capp->fixed_number);
        DBG("  fixed_width   : %d\n", capp->fixed_width);
    }
    
    if (info_p->cpuid_max_basic >= 0xd) {
        cpuid_xsave_leaf_t	*xsp = &info_p->cpuid_xsave_leaf;
        /*
         * XSAVE Features:
         */
        cpuid_fn(0xd, info_p->cpuid_xsave_leaf.extended_state);
        info_p->cpuid_xsave_leafp = xsp;
        
        DBG(" XSAVE Leaf:\n");
        DBG("  EAX           : 0x%x\n", xsp->extended_state[eax]);
        DBG("  EBX           : 0x%x\n", xsp->extended_state[ebx]);
        DBG("  ECX           : 0x%x\n", xsp->extended_state[ecx]);
        DBG("  EDX           : 0x%x\n", xsp->extended_state[edx]);
    }
    
    if (info_p->cpuid_model >= CPUID_MODEL_IVYBRIDGE) {
        /*
         * Leaf7 Features:
         */
        cpuid_fn(0x7, reg);
        info_p->cpuid_leaf7_features = reg[ebx];
        
        DBG(" Feature Leaf7:\n");
        DBG("  EBX           : 0x%x\n", reg[ebx]);
    }
    
    return;
}

static uint32_t
cpuid_set_cpufamily(i386_cpu_info_t *info_p)
{
    uint32_t cpufamily = CPUFAMILY_UNKNOWN;
    
    switch (info_p->cpuid_family) {
        case 6:
            switch (info_p->cpuid_model) {
                case CPUID_MODEL_PENTIUM_M:
                case CPUID_MODEL_ATOM:
                    cpufamily = CPUFAMILY_INTEL_6_13;
                    break;
                    //                    case CPUID_MODEL_YONAH:
                    //                        cpufamily = CPUFAMILY_INTEL_YONAH;
                    //                        break;
                    //                    case CPUID_MODEL_MEROM:
                    //                        cpufamily = CPUFAMILY_INTEL_MEROM;
                    //                        break;
                case CPUID_MODEL_PENRYN:
                    cpufamily = CPUFAMILY_INTEL_PENRYN;
                    break;
                case CPUID_MODEL_NEHALEM:
                case CPUID_MODEL_FIELDS:
                case CPUID_MODEL_DALES:
                case CPUID_MODEL_NEHALEM_EX:
                    cpufamily = CPUFAMILY_INTEL_NEHALEM;
                    break;
                case CPUID_MODEL_DALES_32NM:
                case CPUID_MODEL_WESTMERE:
                case CPUID_MODEL_WESTMERE_EX:
                    cpufamily = CPUFAMILY_INTEL_WESTMERE;
                    break;
                case CPUID_MODEL_SANDYBRIDGE:
                case CPUID_MODEL_JAKETOWN:
                    cpufamily = CPUFAMILY_INTEL_SANDYBRIDGE;
                    break;
                case CPUID_MODEL_IVYBRIDGE:
                case CPUID_MODEL_IVYBRIDGE_EP:
                    cpufamily = CPUFAMILY_INTEL_IVYBRIDGE;
                    break;
                case CPUID_MODEL_HASWELL_DT:
                case CPUID_MODEL_HASWELL_MB:
                case CPUID_MODEL_HASWELL_ULT:
                case CPUID_MODEL_HASWELL_ULX:
                    cpufamily = CPUFAMILY_INTEL_HASWELL;
                    break;
                case CPUID_MODEL_BROADWELL_DT:
                case CPUID_MODEL_BROADWELL_MB:
                case CPUID_MODEL_BROADWELL_ULV:
                    cpufamily = CPUFAMILY_INTEL_BROADWELL;
                    break;
                case CPUID_MODEL_SKYLAKE_LT:
                case CPUID_MODEL_SKYLAKE_DT:
                    cpufamily = CPUFAMILY_INTEL_SKYLAKE;
                    break;
                    
                case CPUID_MODEL_KABYLAKE_U:
                case CPUID_MODEL_KABYLAKE_S:
                    cpufamily = CPUFAMILY_INTEL_KABYLAKE;
            }
            break;
    }
    
    info_p->cpuid_cpufamily = cpufamily;
    DBG("cpuid_set_cpufamily(%p) returning 0x%x\n", info_p, cpufamily);
    return cpufamily;
}

/*
 * Must be invoked either when executing single threaded, or with
 * independent synchronization.
 */
static void
cpuid_set_info(void)
{
    i386_cpu_info_t		*info_p = &cpuid_cpu_info;
    
    cpuid_set_generic_info(info_p);
    
    /* verify we are running on a supported CPU */
    if ((strncmp(CPUID_VID_INTEL, info_p->cpuid_vendor,
                 min(strlen(CPUID_STRING_UNKNOWN) + 1,
                     sizeof(info_p->cpuid_vendor)))) ||
        (cpuid_set_cpufamily(info_p) == CPUFAMILY_UNKNOWN))
        panic("Unsupported CPU");
    
    info_p->cpuid_cpu_type = CPU_TYPE_X86;
    info_p->cpuid_cpu_subtype = CPU_SUBTYPE_X86_ARCH1;
    /* Must be invoked after set_generic_info */
    //cpuid_set_cache_info(info_p);
    
    /*
     * Find the number of enabled cores and threads
     * (which determines whether SMT/Hyperthreading is active).
     */
    switch (info_p->cpuid_cpufamily) {
        case CPUFAMILY_INTEL_WESTMERE: {
            uint64_t msr = rdmsr64(MSR_CORE_THREAD_COUNT);
            info_p->core_count   = bitfield32((uint32_t)msr, 19, 16);
            info_p->thread_count = bitfield32((uint32_t)msr, 15,  0);
            break;
        }
        case CPUFAMILY_INTEL_KABYLAKE:
        case CPUFAMILY_INTEL_SKYLAKE:
        case CPUFAMILY_INTEL_BROADWELL:
        case CPUFAMILY_INTEL_HASWELL:
        case CPUFAMILY_INTEL_IVYBRIDGE:
        case CPUFAMILY_INTEL_SANDYBRIDGE:
        case CPUFAMILY_INTEL_NEHALEM: {
            uint64_t msr = rdmsr64(MSR_CORE_THREAD_COUNT);
            info_p->core_count   = bitfield32((uint32_t)msr, 31, 16);
            info_p->thread_count = bitfield32((uint32_t)msr, 15,  0);
            break;
        }
        default:
        {
            uint32_t	reg[4];
            cpuid_fn(1, reg);
            info_p->core_count = (uint32_t)bitfield32(reg[1], 23, 16);
            info_p->thread_count = info_p->cpuid_logical_per_package;
            break;
        }
    }
    if (info_p->core_count == 0) {
        info_p->core_count   = info_p->cpuid_cores_per_package;
        info_p->thread_count = info_p->cpuid_logical_per_package;
    }
    DBG("cpuid_set_info():\n");
    DBG("  core_count   : %d\n", info_p->core_count);
    DBG("  thread_count : %d\n", info_p->thread_count);
    
    info_p->cpuid_model_string = ""; /* deprecated */
}


#endif /*M_CPUID_H*/
