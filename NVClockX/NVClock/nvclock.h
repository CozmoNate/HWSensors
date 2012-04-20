/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2007 Roderick Colenbrander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef NVCLOCK_H
#define NVCLOCK_H

#include <IOKit/IOLib.h>
#include "FakeSMCDefinitions.h"

#define MAX_CARDS 4

#define NV5  (1<<0)
#define NV10 (1<<1)
#define NV17 (1<<2)
#define NV1X (NV10 | NV17)
#define NV20 (1<<3)
#define NV25 (1<<4)
#define NV2X (NV20 | NV25)
#define NV30 (1<<5)
#define NV31 (1<<6)
#define NV35 (1<<7)
#define NV3X (NV30 | NV31 | NV35)
#define NV40 (1<<8)
#define NV41 (1<<9)
#define NV43 (1<<10)
#define NV44 (1<<11)
#define NV46 (1<<12)
#define NV47 (1<<13)
#define NV49 (1<<14)
#define NV4B (1<<15)
#define C51  (1<<16)
#define NV4X (NV40 | NV41 | NV43 | NV44 | NV46 | NV47 | NV49 | NV4B | C51)
#define NV50 (1<<17)
#define G84 (1<<18)
#define G86 (1<<19)
#define G92 (1<<20)
#define G94 (1<<21)
#define G96 (1<<22)
#define GT200 (1<<23)
#define GF100 (1<<24)
#define NV5X (NV50 | G84 | G86 | G92 | G94 | G96 | GT200 | GF100)

#define NV_ERR_NO_DEVICES_FOUND 1
#define NV_ERR_NO_DRIVERS_FOUND 2
#define NV_ERR_NOT_ENOUGH_PERMISSIONS 3
#define NV_ERR_OTHER 4

#define GPU_OVERCLOCKING (1<<0)
#define MEM_OVERCLOCKING (1<<1)
#define COOLBITS_OVERCLOCKING (1<<2)
#define PIPELINE_MODDING (1<<3)
#define GPU_FANSPEED_MONITORING (1<<4) /* Fanspeed monitoring based on fan voltage */
#define BOARD_TEMP_MONITORING (1<<5) /* Board temperature; not available using NVSensor */
#define GPU_TEMP_MONITORING (1<<6) /* Internal GPU temperature */
#define I2C_FANSPEED_MONITORING (1<<7) /* Fanspeed monitoring using a i2c sensor chip */
#define I2C_AUTOMATIC_FANSPEED_CONTROL (1<<8) /* The sensor supports automatic fanspeed control */
#define SMARTDIMMER (1<<9) /* Smartdimmer support for mobile GPUs */
#define GPU_ID_MODDING (1<<10) /* PCI id modding is supported on this board */

#define STATE_LOWLEVEL (1<<0)
#define STATE_2D (1<<1)
#define STATE_3D (1<<2)
#define STATE_BOTH (STATE_2D | STATE_3D)

/* Define some i2c types, so that we don't depend on additional headers when using NVClock as a library */
#ifndef _XF86I2C_H
#include <xf86i2c.h>
/*typedef void* I2CBusPtr;
typedef void* I2CDevPtr;*/
#endif

typedef struct
{
	void *next;
	char *section;
	char *name;
	unsigned int value;
} cfg_entry;

typedef enum
{
	SDR,
	DDR
} mem_type;

typedef enum
{
	UNKNOWN,
	DESKTOP,
	NFORCE,
	MOBILE
} gpu_type;

struct pci_ids {
	short id;
	const char *name;
	gpu_type gpu;
};

struct voltage
{
	unsigned char VID;
	float voltage;
};

struct performance
{
	int nvclk;
	int delta;
	int memclk;
	int shaderclk;
	int fanspeed;
	float voltage;
};

struct vco
{
	unsigned int minInputFreq, maxInputFreq;
	unsigned int minFreq, maxFreq;
	unsigned char minN, maxN;
	unsigned char minM, maxM;
};

struct pll
{
	unsigned int reg;
	unsigned char var1d;
	unsigned char var1e;
	struct vco VCO1;
	struct vco VCO2;
};

struct sensor
{
	int slope_div;
	int slope_mult;
	int diode_offset_div;
	int diode_offset_mult;
	int temp_correction;
};

struct nvbios
{
	char *signon_msg;
	char *vendor_name;
	unsigned short device_id;
	char* version;
	unsigned char major;
	unsigned char minor;

	short volt_entries;
	short volt_mask;
	struct voltage volt_lst[4];

	short perf_entries;
	struct performance perf_lst[4];

	short pll_entries;
	struct pll pll_lst[16];

	struct sensor sensor_cfg;

	/* Cache the 'empty' PLLs, this is needed for PLL calculation */
	unsigned int mpll;
	unsigned int nvpll;
	unsigned int spll;

	unsigned int pipe_cfg; /* Used to cache the NV4x pipe_cfg register */
};

typedef struct {
	char *card_name; /* Name of the card */
	short number; /* internal card number; used by the gtk client and set_card to see if we really need to switch cards */
	short caps; /* A bitmask that contains what card features are supported; A normal gpu can do gpu/memory overclocking but a nforce can do only gpu. */
	short device_id;
	short subvendor_id;
	int arch; /* Architecture NV10, NV15, NV20 ..; for internal use only as we don't list all architectures */
/*#if __LP64__
    mach_vm_address_t reg_address;
#else
    vm_address_t reg_address;
#endif*/
	//unsigned int reg_address;
	char *dev_name; /* /dev/mem or /dev/nvidiaX */
	unsigned short devbusfn;
	int irq; /* We might need the IRQ to sync NV-CONTROL info with nvclock */
	short base_freq;
	gpu_type gpu; /* Tells what type of gpu is used: mobile, nforce .. */
	short debug; /* Enable/Disable debug information */

	struct nvbios *bios; /* pointer to bios information */
	int have_coolbits; /* Tells if Coolbits (NV-CONTROL) is enabled */
	int state; /* Tells which clocks to change for NV-CONTROL: 2D, 3D or BOTH */

	/* card registers */
	int mem_mapped; /* Check for set_card to see if the memory has been mapped or not. */
	volatile unsigned int *PFB;
	volatile unsigned int *PBUS;
	volatile unsigned int *PDISPLAY; /* NV50 display registers */
	volatile unsigned int *PMC;
	volatile unsigned int *PRAMDAC;
	volatile unsigned int *PRAMIN;
	volatile unsigned int *PEXTDEV;
	volatile unsigned char *PROM; /* Nvidia bios */
	volatile unsigned char *PCIO;

	/* Overclock range of speeds */
	short nvclk_min; 
	short nvclk_max; 
	short memclk_min; 
	short memclk_max; 

	/* Various GeforceFX/Geforce6 boards use different clocks in 3d. We need to store those clocks */
	short nvclk_3d;
	short memclk_3d;

	/* Card info */
	short (*get_gpu_architecture)();
	short (*get_gpu_revision)();
	int   (*set_gpu_pci_id)(short id); /* Changes the least significant 2 or 4 bits of the pci id */

	/* Memory info */
	int mem_type; /* needs to replace memory_type ?? */
	char* (*get_memory_type)(); /* Memory type: SDR/DDR */
	short (*get_memory_width)(); /* Memory width 64bit or 128bit */
	short (*get_memory_size)(); /* Amount of memory between 4 and 128 MB */

	/* BUS info: PCI/PCI-Express/AGP */
	char* (*get_bus_type)(); /* Bus type: AGP/PCI/PCI-Express */
	short (*get_bus_rate)(); /* Current AGP rate: 1, 2, 4  or 8; PCI-Express: 1-32X*/

	/* AGP */
	char* (*get_agp_status)(); /* Current AGP status: Enabled/Disabled */
	char* (*get_agp_fw_status)(); /* Current FW status: Enabled/Disabled */
	char* (*get_agp_sba_status)(); /* Current SBA status: Enabled/Disabled */
	char* (*get_agp_supported_rates)(); /* Supported AGP rates */
	
	/* PCI-Express */
	short (*get_pcie_max_bus_rate)(); /* Get the maximum PCI-E busrate */

	/* Hardware monitoring */
	short num_busses; /* Number of available i2c busses */
	I2CBusPtr busses[4]; /* I2C busses on the videocard; this bus is needed for communication with sensor chips */
	I2CDevPtr sensor; /* When a sensor chip is available, this device pointer can be used to access it */
	char *sensor_name; /* Name of the sensor; although sensor contains the name too, we add sensor_name because of the builtin temperature sensor used on various NV4x cards */
	int (*get_board_temp)(I2CDevPtr dev); /* Temperature of the sensor chip or for example the ram chips */
	int (*get_gpu_temp)(I2CDevPtr dev); /* Internal gpu temperature */
	float (*get_fanspeed)(); /* Get the speed in % from the pwm register in %*/
	void (*set_fanspeed)(float speed); /* Set the speed of the fan using the pwm register of the gpu */
	int (*get_i2c_fanspeed_mode)(I2CDevPtr dev); /* Advanced sensors like the ADT7473 support manual and automatic fanspeed adjustments */
	void (*set_i2c_fanspeed_mode)(I2CDevPtr dev, int mode); /* Set the fanspeed mode to manual or automatic; Note that a pwm fanspeed change already causes a switch to implicit switch to manual, so this function should only be used to deactivate manual mode */
	int (*get_i2c_fanspeed_rpm)(I2CDevPtr dev); /* Speed of the fan in rpm */
	float (*get_i2c_fanspeed_pwm)(I2CDevPtr dev); /* Duty cycle of the pwm signal that controls the fan */
	int (*set_i2c_fanspeed_pwm)(I2CDevPtr dev, float speed); /* By adjusting the duty cycle of the pwm signal, the fanspeed can be adjusted. */

	/* Pipeline stuff for NV4x; On various Geforce6 boards disabled pixel/vertex pipelines can be re-enabled. */
	int (*get_default_mask)(char *pmask, char *vmask);
	int (*get_hw_masked_units)(char *pmask, char *vmask);
	int (*get_sw_masked_units)(char *pmask, char *vmask);
	int (*get_pixel_pipelines)(char *mask, int *total);
	void (*set_pixel_pipelines)(unsigned char mask);
	int (*get_vertex_pipelines)(char *mask);
	void (*set_vertex_pipelines)(unsigned char mask);

	/* NV5x (Geforce8) shader stuff */
	float (*get_shader_speed)(); /* NV5X-only */
	void (*set_shader_speed)(unsigned int clk); /* NV5X-only */
	void (*reset_shader_speed)();
	int (*get_stream_units)(char *mask, char *default_mask);
	int (*get_rop_units)(char *mask, char *default_mask);

	/* Smartdimmer (adjustment of the brigthenss of the backlight on Laptops) */
	int (*get_smartdimmer)();
	void (*set_smartdimmer)(int level);

	/* Overclocking */
	volatile unsigned int mpll; /* default memory speed */
	volatile unsigned int mpll2;
	volatile unsigned int nvpll; /* default gpu speed */
	volatile unsigned int nvpll2;

	void (*set_state)(int state);
	float (*get_gpu_speed)();
	void (*set_gpu_speed)(unsigned int nvclk);
	float (*get_memory_speed)();
	void (*set_memory_speed)(unsigned int memclk);
	void (*reset_gpu_speed)();
	void (*reset_memory_speed)();

	/* Debug */
	void (*get_debug_info)();
} NVCard, *NVCardPtr;

typedef struct
{
	int num_cards;
	NVCard card[MAX_CARDS];
	cfg_entry *cfg;
	void *dpy; /* X display needed for the NV-CONTROL backend */
	char *path; /* path to home directory */
	int nv_errno;
	char *nv_err_str;
} NVClock;

extern NVClock nvclock;
extern NVCard* nv_card;

#ifdef __cplusplus
extern "C" {
#endif

int init_nvclock();
int set_card(int number);
void unset_card();


/* error handling */
char *get_error(char *buf, int size);
void set_error(int code);
void set_error_str(const char *err);

/* utility functions */
int convert_gpu_architecture(short arch, char *buf);
void convert_unit_mask_to_binary(char mask, char hw_default, char *buf);

#ifdef __cplusplus
};
#endif

#endif /* NVCLOCK_H */
