/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 * 
 * Copyright(C) 2001-2005 Roderick Colenbrander
 *
 * site: http://nvclock.sourceforge.net
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

//#include "config.h"
#include "i2c.h"
#include "nvclock.h"
#include "nvreg.h"
#include <stdint.h>


/* Thanks to Alexey Nicolaychuk (Unwinder), the author of Rivatuner, for providing
   these macros to get nv30 clock detection working.
*/
#define NV30_PLL_M1(PLL) ( ( PLL ) & 0x0f )
#define NV30_PLL_M2(PLL) ( ( ( PLL )  >>  4 ) & 0x07 )
#define NV30_PLL_N1(PLL) ( ( ( PLL ) >> 8 ) & 0xff )
#define NV30_PLL_N2(PLL) ( ( ( ( PLL ) >> 19 ) & 0x07 ) | ( ( ( PLL ) >> 21 ) & 0x18 ) )
#define NV30_PLL_P(PLL) ( ( ( PLL ) >> 16 ) & 0x07 )

#define PCI_GET_BUS(devbusfn) ((devbusfn >> 8) & 0xff)
#define PCI_GET_DEVICE(devbusfn) ((devbusfn & 0xff) >> 3)
#define PCI_GET_FUNCTION(devbusfn) (devbusfn & 0x7)
#define PCI_GET_DEVBUSFN(dev, bus, fn) ((bus << 8) | (dev << 3) | (fn & 0x7))

/* Set the card object to the requested card */
int set_card(int number);

/* Some internally needed functions */
const char* get_card_name(int device_id, gpu_type *gpu);
int get_gpu_arch(int device_id);
int set_card_info(int number); /* Basic version of set_card */
//int map_mem(const char* dev_name);
//void unmap_mem();
int32_t pciReadLong(unsigned short devbusfn, long offset);

/* Bios related stuff */
void dump_bios(const char *filename);
struct nvbios* read_bios(const char *filename);

/* NV-CONTROL overclocking functions */
float nvcontrol_get_gpu_speed();
float nvcontrol_get_memory_speed(); /* NV-CONTROL wrapper */
void nvcontrol_set_gpu_speed(unsigned int nvclk);
void nvcontrol_set_memory_speed(unsigned int memclk);
void nvcontrol_reset_gpu_speed();
void nvcontrol_reset_memory_speed();

/* PLL to clock conversion */
float GetClock(int base_freq, unsigned int pll);
float GetClock_nv30(int base_freq, unsigned int pll, unsigned int pll2);
float GetClock_nv40(int base_freq, unsigned int pll, unsigned int pll2);
float GetClock_nv50(int base_freq, unsigned int pll, unsigned int pll2);

void info_init(void);
void i2c_sensor_init(void);

void nv_init(void);
void nv30_init(void);
void nv31_init(void);
void nv40_init(void);
void nv50_init(void);

/* PCI bus reading */
unsigned char nv_read_pbus8(int offset);
unsigned short nv_read_pbus16(int offset);
unsigned int nv_read_pbus(int offset);

/* PMC reading */
unsigned int nv_read_pmc(int offset);

