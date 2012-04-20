/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * Copyright(C) 2001-2007 Roderick Colenbrander
 *
 * Copyright(C) 2005 Hans-Frieder Vogt
 * NV40 bios parsing improvements (BIT parsing rewrite + performance table fixes)
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

/*
TODO:
- support for parsing some other init/script tables
- support for pre-GeforceFX bioses
*/

#include "backend.h"
#include "nvclock.h"
#include "nvreg.h"
//#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
//#include <unistd.h>
//#include <stdio.h>
#include <stdint.h>
//#include <stdlib.h>
#include <string.h>

#define READ_BYTE(rom, offset)  (rom[offset]&0xff)
#define READ_SHORT(rom, offset) ((rom[offset+1]&0xff) << 8 | (rom[offset]&0xff))
#define READ_INT(rom, offset) ((rom[offset+3]&0xff) << 24 | (rom[offset+2]&0xff) << 16 | (rom[offset+1]&0xff) << 8 | (rom[offset]&0xff))
#define READ_LONG(rom, offset) (READ_INT(rom, offset+4)<<32 | READ_INT(rom, offset))

//static unsigned int locate(char *rom, char *str, int offset);
//struct nvbios *read_bios(const char *data);
//static struct nvbios *parse_bios(char *rom);
//int load_bios_prom(char *data);

typedef struct
{
	unsigned char version;
	unsigned char start;
	unsigned char entry_size;
	unsigned char num_entries;
} BitTableHeader;

typedef struct
{
	unsigned char version;
	unsigned char start;
	unsigned char num_active_entries;
	unsigned char offset;
	unsigned char entry_size;
	unsigned char num_entries;
} BitPerformanceTableHeader;

/* Read a string from a given offset */
static char* nv_read(char *rom, unsigned short offset)
{
	/*//char *res = STRDUP(&rom[offset], (NV_PROM_SIZE - offset) + 1);
	size_t len = strlen(&rom[offset]);
	size_t end = len;

// Currently we only use this function for reading the signon message.
// The string ends with a '\n' which we don't want so remove it.

	for(size_t i=0; i < len; i++)
		if(rom[i] == '\n' || rom[i] == '\r') {
			//rom[i] = '\0';
            end = i;
        }
    
    return STRDUP(&rom[offset], end);*/
    return STRDUP("", sizeof(""));
}


static char *bios_version_to_str(int version)
{
	char res[12];
	snprintf(res,12, "%02x.%02x.%02x.%02x%c", (version>>24) & 0xff, (version>>16) & 0xff, (version>>8) & 0xff, version&0xff, '\0');
	return (char*)STRDUP(res,12);
}


/* Parse the GeforceFX performance table */
static void parse_nv30_performance_table(struct nvbios *bios, char *rom, int offset)
{
	short i, num_entries;
	unsigned char start;
	unsigned char size;
	//int tmp = 0;

	/* read how far away the start is */
	start = rom[offset];
	num_entries = rom[offset+2];
	size = rom[offset + 3];

	bios->perf_entries=0;
	offset += start + 1;
	for(i=0; i < num_entries; i++)
	{
		bios->perf_lst[i].nvclk =  (READ_INT(rom, offset))/100;

		/* The list can contain multiple distinct memory clocks.
		/  Later on the ramcfg register can tell which of the ones is the right one.
		/  But for now assume the first one is correct. It doesn't matter much if the
		/  clocks are a little lower/higher as we mainly use this to detect 3d clocks
		/
		/  Further the clock stored here is the 'real' memory frequency, the effective one
		/  is twice as high. It doesn't seem to be the case for all bioses though. In some effective
		/  and real speed entries existed but this might be patched dumps.
		*/
		bios->perf_lst[i].memclk =  (READ_INT(rom, offset+4))/50;

		/* Move behind the timing stuff to the fanspeed and voltage */
		bios->perf_lst[i].fanspeed = (float)(unsigned char)rom[offset + 54];
		bios->perf_lst[i].voltage = (float)(unsigned char)rom[offset + 55]/100;
		/* In case the voltage is 0, assume the voltage is similar to the previous voltage */
		if(bios->perf_lst[i].voltage==0 && i>0)
			bios->perf_lst[i].voltage = bios->perf_lst[i-1].voltage;

		bios->perf_entries++;
		offset += size;
	}
}


/* Convert the bios version which is stored in a numeric way to a string.
/  On NV40 bioses it is stored in 5 numbers instead of 4 which was the
/  case on old cards. The bios version on old cards could be bigger than
/  4 numbers too but that version was only stored in a string which was
/  hard to locate. On NV40 cards the version is stored in a string too,
/  for which the offset can be found at +3 in the 'S' table.
*/
static char *nv40_bios_version_to_str(char *rom, short offset)
{
	char res[16];
	int version = READ_INT(rom, offset);
	unsigned char extra = rom[offset+4];

	snprintf(res, 16,"%02X.%02x.%02x.%02x.%02x%c", (version>>24) & 0xff, (version>>16) & 0xff, (version>>8) & 0xff, version&0xff, extra, '\0');
	 return (char*)STRDUP(res,16);
}


/* Init script tables contain dozens of entries containing commands to initialize
/  the card. There are lots of different commands each having a different 'id' useally
/  most entries also have a different size. The task of this function is to move to the
/  next entry in the table.
*/
static int bit_init_script_table_get_next_entry(char *rom, int offset)
{
	unsigned char id = rom[offset];
	//int i=0;

	switch(id)
	{
		case '2': /* 0x32 */
			offset += 43;
			break;
		case '3': /* 0x33: INIT_REPEAT */
			offset += 2;
			break;
		case '6': /* 0x36: INIT_REPEAT_END */
			offset += 1;
			break;
		case '7': /* 0x37 */
			offset += 11;
			break;
		case '8': /* 0x38: INIT_NOT */
			offset += 1;
			break;
		case '9': /* 0x39 */
			offset += 2;
			break;
		case 'J': /* 0x4A */
			offset += 43;
			break;
		case 'K': /* 0x4B */
#if Debug
			/* +1 = PLL register, +5 = value */
			IOLog("'%c'\t%08x %08x\n", id, READ_INT(rom, offset+1), READ_INT(rom, offset+5));
#endif
			offset += 9;
			break;
		case 'M': /* 0x4D: INIT_ZM_I2C_BYTE */
#if Debug
			IOLog("'%c'\ti2c bytes: %x\n", id, rom[offset+3]);
#endif
			offset += 4 + rom[offset+3]*2;
			break;
		case 'Q': /* 0x51 */
			offset += 5 + rom[offset+4];
			break;
		case 'R': /* 0x52 */
			offset += 4;
			break;
		case 'S': /* 0x53: INIT_ZM_CR */
#if Debug
			/* +1 CRTC index (8-bit)
			/  +2 value (8-bit)
			*/
			IOLog("'%c'\tCRTC index: %x value: %x\n", id, rom[offset+1], rom[offset+2]);
#endif
			offset += 3;
			break;
		case 'T': /* 0x54 */
			offset += 2 + rom[offset+1] * 2;
			break;
		case 'V': /* 0x56 */
			offset += 3;
			break;
		case 'X': /* 0x58 */
#if Debug
			{
				/* +1 register base (32-bit)
				/  +5 number of values (8-bit)
				/  +6 value (32-bit) to regbase+4
				/  .. */
				int base = READ_INT(rom, offset+1);
				int number = (unsigned char)rom[offset+5];

				IOLog("'%c'\tbase: %08x number: %d\n", id, base, number);
				for(int i=0; i<number; i++)
					IOLog("'%c'\t %08x: %08x\n", id, base+4*i, READ_INT(rom, offset+6 + 4*i));
			}
#endif
			offset += 6 + rom[offset+5] * 4;
			break;
		case '[': /* 0x5b */
			offset += 3;
			break;
		case '_': /* 0x5F */
			offset += 22;
			break;
		case 'b': /* 0x62 */
			offset += 5;
			break;
		case 'c': /* 0x63 */
			offset +=1;
			break;
		case 'e': /* 0x65: INIT_RESET */
#if Debug
			/* +1 register (32-bit)
			/  +5 value (32-bit)
			/  +9 value (32-bit)
			*/
			IOLog("'%c'\t%08x %08x %08x\n", id, READ_INT(rom, offset+1), READ_INT(rom, offset+5), READ_INT(rom, offset+9));
#endif
			offset += 13;
			break;
		case 'i': /* 0x69 */
			offset += 5;
			break;
		case 'k': /* 0x6b: INIT_SUB */
#if Debug
			IOLog("'%c' executing SUB: %x\n", id, rom[offset+1]);
#endif
			offset += 2;
			break;
		case 'n': /* 0x6e */
#if Debug
			/* +1 = register, +5 = AND-mask, +9 = value */
			IOLog("'%c'\t%08x %08x %08x\n", id, READ_INT(rom, offset+1), READ_INT(rom, offset+5), READ_INT(rom, offset+9));
#endif
			offset += 13;
			break;
		case 'o': /* 0x6f */
			offset += 2;
			break;
		case 'q': /* 0x71: quit */
			offset += 1;
			break;
		case 'r': /* 0x72: INIT_RESUME */
			offset += 1;
			break;
		case 't': /* 0x74 */
			offset += 3;
			break;
		case 'u': /* 0x75: INIT_CONDITION */
#if Debug
			IOLog("'%c'\t condition: %d\n", id, rom[offset+1]);
#endif
			offset += 2;
			break;
		case 'v': /* 0x76: INIT_IO_CONDITION */
#if Debug
			IOLog("'%c'\t IO condition: %d\n", id, rom[offset+1]);
#endif
			offset += 2;
			break;
		case 'x': /* 0x78: INIT_INDEX_IO */
#if Debug
			/* +1 CRTC reg (16-bit)
			/  +3 CRTC index (8-bit)
			/  +4 AND-mask (8-bit)
			/  +5 OR-with (8-bit)
			*/
			IOLog("'%c'\tCRTC reg: %x CRTC index: %x AND-mask: %x OR-with: %x\n", id, READ_SHORT(rom, offset+1), rom[offset+3], rom[offset+4], rom[offset+5]);
#endif
			offset += 6;
			break;
		case 'y': /* 0x79 */
#if Debug
			/* +1 = register, +5 = clock */
			IOLog("'%c'\t%08x %08x (%dMHz)\n", id, READ_INT(rom, offset+1), READ_SHORT(rom, offset+5), READ_SHORT(rom, offset+5)/100);
#endif
			offset += 7;
			break;
		case 'z': /* 0x7a: INIT_ZM_REG */
#if Debug
			/* +1 = register, +5 = value */
			IOLog("'%c'\t%08x %08x\n", id, READ_INT(rom, offset+1), READ_INT(rom, offset+5));
#endif
			offset += 9;
			break;
		case 0x8e: /* 0x8e */
			/* +1 what is this doing? */
			offset += 1;
			break;
		case 0x8f: /* 0x8f: INIT_ZM_REG */
#if Debug
			{
				/* +1 register 
				/  +5 = length of sequence (?)
				/  +6 = num entries
				*/
				int size = rom[offset+5];
				int number = rom[offset+6];
				IOLog("'%c'\treg: %08x size: %d number: %d", id, READ_INT(rom, offset+1), size, number);
				/* why times 2? */
				for(int i=0; i<number*size*2; i++)
					IOLog(" %08x", READ_INT(rom, offset + 7 + i));
				IOLog("\n");
			}
#endif
			offset += READ_BYTE(rom, offset+6) * 32 + 7;
			break;
		case 0x90: /* 0x90 */
			offset += 9;
			break;
		case 0x91: /* 0x91 */
#if Debug
			/* +1 = pll register, +5 = ?, +9 = ?, +13 = ? */
			IOLog("'%c'\t%08x %08x\n", id, READ_INT(rom, offset+1), READ_INT(rom, offset+5));
#endif
			offset += 18;
			break;
		case 0x97: /* 0x97 */
#if Debug
			IOLog("'%c'\t%08x %08x\n", id, READ_INT(rom, offset+1), READ_INT(rom, offset+5));
#endif
			offset += 13;
			break;
		default:
#if Debug
			IOLog("Unhandled init script entry with id '%c' at %04x", id, offset);
#endif
			return 0;
	}

	return offset;
}


static void parse_bit_init_script_table(struct nvbios *bios, char *rom, int init_offset, int len)
{
	int /*i,*/offset;
	//int done=0;
	unsigned char id;

	/* Table 1 */
	offset = READ_SHORT(rom, init_offset);

	/* For pipeline modding purposes we cache 0x1540 and for PLL generation the PLLs */
	id = rom[offset];
	while(id != 'q')
	{
		offset = bit_init_script_table_get_next_entry(rom, offset);
		/* Break out of the loop if we find an unknown entry id */
		if(!offset)
			break;
		id = rom[offset];

		if(id == 'z')
		{
			int reg = READ_INT(rom, offset+1);
			unsigned int val = READ_INT(rom, offset+5);
			switch(reg)
			{
				case 0x1540:
					bios->pipe_cfg = val;
					break;
				case 0x4000:
					bios->nvpll = val;
					break;
				case 0x4020:
					bios->mpll = val;
					break;
			}
		}
	}

#if Debug /* Read all init tables and print some debug info */
/* Table 1 */
	offset = READ_SHORT(rom, init_offset);

	for(int i=0; i<=len; i+=2)
	{
		/* Not all tables have to exist */
		if(!offset)
		{
			init_offset += 2;
			offset = READ_SHORT(rom, init_offset);
			continue;
		}

		IOLog("Init script table %d\n", i/2+1);
		id = rom[offset];

		while(id != 'q')
		{
			/* Break out of the loop if we find an unknown entry id */
			if(!offset)
				break;

			if(!(id == 'K' || id == 'n' || id == 'x' || id == 'y' || id == 'z'))
				IOLog("'%c' (%x)\n", id, id);
			offset = bit_init_script_table_get_next_entry(rom, offset);
			id = rom[offset];
		}

		/* Pointer to next init table */
		init_offset += 2;
		/* Get location of next table */
		offset = READ_SHORT(rom, init_offset);
	}
#endif

}


/* Parse the Geforce6/7/8 performance table */
static void parse_bit_performance_table(struct nvbios *bios, char *rom, int offset)
{
	short i, entry;
	unsigned char entry_size;
	short nvclk_offset, memclk_offset, shader_offset, fanspeed_offset, voltage_offset;
	BitPerformanceTableHeader *header = (BitPerformanceTableHeader*)&rom[offset];

	/* The first byte contains a version number; based on this we set offsets to interesting entries */
	switch(header->version)
	{
		case 0x25: /* First seen on Geforce 8800GTS bioses */
			fanspeed_offset = 4;
			voltage_offset = 5;
			nvclk_offset = 8;
			shader_offset = 10;
			memclk_offset = 12;
			break;
		case 0x30: /* First seen on Geforce 8600GT bioses */
			fanspeed_offset = 6;
			voltage_offset = 7;
			nvclk_offset = 8;
			shader_offset = 10;
			memclk_offset = 12;
			break;
		case 0x35: /* First seen on Geforce 8800GT bioses; what else is different? */
			fanspeed_offset = 6;
			voltage_offset = 7;
			nvclk_offset = 8;
			shader_offset = 10;
			memclk_offset = 12;
			break;
		default: /* Default to this for all other bioses, I haven't seen issues yet for the entries we use */
			shader_offset = 0;
			fanspeed_offset = 4;
			voltage_offset = 5;
			nvclk_offset = 6;
			memclk_offset = 11;
	}

	/* +5 contains the number of entries, +4 the size of one in bytes and +3 is some 'offset' */
	entry_size = header->offset + header->entry_size * header->num_entries;

	/* now read entries
	/  entries start with 0x20 for entry 0, 0x21 for entry 1, ...
	*/
	offset += header->start;

	for(entry=0, i=0; entry<header->num_active_entries; entry++)
	{
		/* On bios version 0x35, this 0x20, 0x21 .. pattern doesn't exist anymore; do the last 4 bits of the first byte tell if an entry is active on 0x35? */
		if ( (header->version != 0x35) && (rom[offset] & 0xf0) != 0x20)
		{
			break;
		}

		bios->perf_lst[i].fanspeed = (unsigned char)rom[offset+fanspeed_offset];

		bios->perf_lst[i].voltage = (float)(unsigned char)rom[offset+voltage_offset]/100;
		/* In case the voltage is 0, assume the voltage is similar to the previous voltage */
		if(bios->perf_lst[i].voltage==0 && i>0)
			bios->perf_lst[i].voltage = bios->perf_lst[i-1].voltage;

		/* HACK: My collection of bioses contains a (valid) 6600 bios with two 'bogus' entries at 0x21 (100MHz) and 0x22 (200MHz)
		/  these entries aren't the default ones for sure, so skip them until we have a better entry selection algorithm.
		*/
		if(READ_SHORT(rom, offset+nvclk_offset) > 200)
		{
			bios->perf_lst[i].nvclk = READ_SHORT(rom, offset+nvclk_offset);

			/* Support delta clock reading on some NV4X boards. The entries seem to be present on most Geforce7 boards but are as far as I know only used on 7800/7900 boards.
			/ On other boards the delta clocks are set to 0. Offset +8 contains the actual delta clock and offset +7 contains a divider for it. If the divider is 0 we don't read the delta clock. */
			if((get_gpu_arch(bios->device_id) & (NV47 | NV49)) && rom[offset+7])
			{
				bios->perf_lst[i].delta = rom[offset+8]/rom[offset+7];
				bios->perf_lst[i].memclk = READ_SHORT(rom, offset+memclk_offset);
			}
			/* Geforce8 cards have a shader clock, further the memory clock is at a different offset as well */
			else if(get_gpu_arch(bios->device_id) & NV5X)
			{
				bios->perf_lst[i].shaderclk= READ_SHORT(rom, offset+shader_offset);
				bios->perf_lst[i].memclk = READ_SHORT(rom, offset+memclk_offset);
			}
			else
				bios->perf_lst[i].memclk = READ_SHORT(rom, offset+memclk_offset)*2;

			bios->perf_entries = i+1;
			i++;
		}
		offset += entry_size;
	}
}

/* Parse the table containing pll programming limits */
static void parse_bit_pll_table(struct nvbios *bios, char *rom, unsigned short offset)
{
	BitTableHeader *header = (BitTableHeader*)&rom[offset];
	int i;

	offset += header->start;
	for(i=0; i<header->num_entries; i++)
	{
		/* Each type of pll (corresponding to a certain register) has its own limits */
		bios->pll_lst[i].reg = READ_INT(rom, offset);

		/* Minimum/maximum frequency each VCO can generate */
		bios->pll_lst[i].VCO1.minFreq = READ_SHORT(rom, offset+4)*1000;
		bios->pll_lst[i].VCO1.maxFreq = READ_SHORT(rom, offset+6)*1000;
		bios->pll_lst[i].VCO2.minFreq = READ_SHORT(rom, offset+8)*1000;
		bios->pll_lst[i].VCO2.maxFreq = READ_SHORT(rom, offset+0xa)*1000;

		/* Minimum/maximum input frequency for each VCO */
		bios->pll_lst[i].VCO1.minInputFreq = READ_SHORT(rom, offset+0xc)*1000;
		bios->pll_lst[i].VCO1.maxInputFreq = READ_SHORT(rom, offset+0xe)*1000;
		bios->pll_lst[i].VCO2.minInputFreq = READ_SHORT(rom, offset+0x10)*1000;
		bios->pll_lst[i].VCO2.maxInputFreq = READ_SHORT(rom, offset+0x12)*1000;

		/* Low and high values for the dividers and multipliers */
		bios->pll_lst[i].VCO1.minN = READ_BYTE(rom, offset+0x14);
		bios->pll_lst[i].VCO1.maxN = READ_BYTE(rom, offset+0x15);
		bios->pll_lst[i].VCO1.minM = READ_BYTE(rom, offset+0x16);
		bios->pll_lst[i].VCO1.maxM = READ_BYTE(rom, offset+0x17);
		bios->pll_lst[i].VCO2.minN = READ_BYTE(rom, offset+0x18);
		bios->pll_lst[i].VCO2.maxN = READ_BYTE(rom, offset+0x19);
		bios->pll_lst[i].VCO2.minM = READ_BYTE(rom, offset+0x1a);
		bios->pll_lst[i].VCO2.maxM = READ_BYTE(rom, offset+0x1b);

		bios->pll_lst[i].var1d = READ_BYTE(rom, offset+0x1d);
		bios->pll_lst[i].var1e = READ_BYTE(rom, offset+0x1e);

#if Debug
		IOLog("register: %#08x\n", READ_INT(rom, offset));

		/* Minimum/maximum frequency each VCO can generate */
		IOLog("minVCO_1: %d\n", READ_SHORT(rom, offset+4));
		IOLog("maxVCO_1: %d\n", READ_SHORT(rom, offset+6));
		IOLog("minVCO_2: %d\n", READ_SHORT(rom, offset+8));
		IOLog("maxVCO_2: %d\n", READ_SHORT(rom, offset+0xa));

		/* Minimum/maximum input frequency for each VCO */
		IOLog("minVCO_1_in: %d\n", READ_SHORT(rom, offset+0xc));
		IOLog("minVCO_2_in: %d\n", READ_SHORT(rom, offset+0xe));
		IOLog("maxVCO_1_in: %d\n", READ_SHORT(rom, offset+0x10));
		IOLog("maxVCO_2_in: %d\n", READ_SHORT(rom, offset+0x12));

		/* Low and high values for the dividers and multipliers */
		IOLog("N1_low: %d\n", READ_BYTE(rom, offset+0x14));
		IOLog("N1_high: %d\n", READ_BYTE(rom, offset+0x15));
		IOLog("M1_low: %d\n", READ_BYTE(rom, offset+0x16));
		IOLog("M1_high: %d\n", READ_BYTE(rom, offset+0x17));
		IOLog("N2_low: %d\n", READ_BYTE(rom, offset+0x18));
		IOLog("N2_high: %d\n", READ_BYTE(rom, offset+0x19));
		IOLog("M2_low: %d\n", READ_BYTE(rom, offset+0x1a));
		IOLog("M2_high: %d\n", READ_BYTE(rom, offset+0x1b));

		/* What's the purpose of these? */
		IOLog("1c: %d\n", READ_BYTE(rom, offset+0x1c));
		IOLog("1d: %d\n", READ_BYTE(rom, offset+0x1d));
		IOLog("1e: %d\n", READ_BYTE(rom, offset+0x1e));
		IOLog("\n");
#endif

		bios->pll_entries = i+1;
		offset += header->entry_size;
	}
}

/* The internal gpu sensor most likely consists of a diode and a resistor.
/  The voltage across this resistor is meassured using a ADC. Since the
/  voltage-current relationship of a diode isn't linear the value needs some correction.
/  The temperature can be calculated by scaling the output value of the ADC and adding an offset
/  to it. 
/
/  This function reads the temperature table and reads the offset/scaling constants for the
/  temperature calculation formula. Before I didn't know where and how these values were stored and
/  used some hardcoded (wrong) values. I expected the values to be tored near the place where
/  the temperature sensor enable/disable bit was but I didn't have the time to figure it all out.
/  The code below is very similar to the code from the Rivatuner gpu diode by Alexey Nicolaychuk with a few adjustments.
/  Rivatuner's code didn't contain constants for the latest Geforce7 (NV46/NV49/NV4B) cards so I had to add those myself.
*/
static void parse_bit_temperature_table(struct nvbios *bios, char *rom, int offset)
{
	short i;
	BitTableHeader *header = (BitTableHeader*)&rom[offset];

	switch(get_gpu_arch(bios->device_id))
	{
		case NV43:
			bios->sensor_cfg.diode_offset_mult = 32060;
			bios->sensor_cfg.diode_offset_div = 1000;
			bios->sensor_cfg.slope_mult = 792;
			bios->sensor_cfg.slope_div = 1000;
			break;
		case NV44:
		case NV47:
			bios->sensor_cfg.diode_offset_mult = 27839;
			bios->sensor_cfg.diode_offset_div = 1000;
			bios->sensor_cfg.slope_mult = 780;
			bios->sensor_cfg.slope_div = 1000;
			break;
		case NV46: /* are these really the default ones? they come from a 7300GS bios */
			bios->sensor_cfg.diode_offset_mult = -24775;
			bios->sensor_cfg.diode_offset_div = 100;
			bios->sensor_cfg.slope_mult = 467;
			bios->sensor_cfg.slope_div = 10000;
			break;
		case NV49: /* are these really the default ones? they come from a 7900GT/GTX bioses */
			bios->sensor_cfg.diode_offset_mult = -25051;
			bios->sensor_cfg.diode_offset_div = 100;
			bios->sensor_cfg.slope_mult = 458;
			bios->sensor_cfg.slope_div = 10000;
			break;
		case NV4B: /* are these really the default ones? they come from a 7600GT bios */
			bios->sensor_cfg.diode_offset_mult = -24088;
			bios->sensor_cfg.diode_offset_div = 100;
			bios->sensor_cfg.slope_mult = 442;
			bios->sensor_cfg.slope_div = 10000;
			break;
	}

	offset += header->start;
	for(i=0; i<header->num_entries; i++)
	{
		unsigned char id = rom[offset];
		short value = READ_SHORT(rom, offset+1);
		
		switch(id)
		{
			/* The temperature section can store settings for more than just the builtin sensor.
			/  The value of 0x0 sets the channel for which the values below are meant. Right now
			/  we ignore this as we only use option 0x10-0x13 which are specific to the builtin sensor.
			/  Further what do 0x33/0x34 contain? Those appear on Geforce7300/7600/7900 cards.
			*/
			case 0x1:
#if Debug
				IOLog("0x1: (%0x) %d 0x%0x\n", value, (value>>9) & 0x7f, value & 0x3ff);
#endif
				if((value & 0x8f) == 0)
					bios->sensor_cfg.temp_correction = (value>>9) & 0x7f;
				break;
			/* An id of 4 seems to correspond to a temperature threshold but 5, 6 and 8 have similar values, what are they? */
			case 0x4:
			case 0x5:
			case 0x6:
			case 0x8:
				/* IOLog("0x%x: 0x%x %d\n", id, value & 0xf, (value>>4) & 0x1ff); */
				break;
			case 0x10:
				bios->sensor_cfg.diode_offset_mult = value;
				break;
			case 0x11:
				bios->sensor_cfg.diode_offset_div = value;
				break;
			case 0x12:
				bios->sensor_cfg.slope_mult = value;
				break;
			case 0x13:
				bios->sensor_cfg.slope_div = value;
				break;
#if Debug
			default:
				IOLog("0x%x: %x\n", id, value);
#endif
		}
		offset += header->entry_size;
	}
#if Debug
	IOLog("temperature table version: %#x\n", header->version);
	IOLog("correction: %d\n", bios->sensor_cfg.temp_correction);
	IOLog("offset: %.3f\n", (float)bios->sensor_cfg.diode_offset_mult / (float)bios->sensor_cfg.diode_offset_div);
	IOLog("slope: %.3f\n", (float)bios->sensor_cfg.slope_mult / (float)bios->sensor_cfg.slope_div);
#endif
}

/* Read the voltage table for nv30/nv40/nv50 cards */
static void parse_voltage_table(struct nvbios *bios, char *rom, int offset)
{
	unsigned char entry_size=0;
	unsigned char start=0;
	int i;

	/* In case of the first voltage table revision, there was no start pointer? */
	switch(rom[offset])
	{
		case 0x10:
		case 0x12:
 			start = 5;
			entry_size = rom[offset+1];
			bios->volt_entries = rom[offset+2];
			bios->volt_mask = rom[offset+4];
			break;
		default:
			start = rom[offset+1];
			bios->volt_entries = rom[offset+2];
			entry_size = rom[offset+3];

			/* The VID mask is stored right before the start of the first entry? */
			bios->volt_mask = rom[offset+start -1];
	}

	offset += start;
	for(i=0; i<bios->volt_entries; i++)
	{
		/* The voltage is stored in multiples of 10mV, scale it to V */
		bios->volt_lst[i].voltage = (float)(unsigned char)rom[offset] / 100;
		bios->volt_lst[i].VID = rom[offset + 1];
		offset += entry_size;
	}
}


static void nv5_parse(struct nvbios *bios, char *rom, unsigned short nv_offset)
{
	/* Go to the position containing the offset to the card name, it is 30 away from NV. */
	int offset = READ_SHORT(rom, nv_offset + 30);
	bios->signon_msg = nv_read(rom, offset);
}


static void nv30_parse(struct nvbios *bios, char *rom, unsigned short nv_offset)
{
	unsigned short init_offset = 0;
	unsigned short perf_offset=0;
	unsigned short volt_offset=0;

	int offset = READ_SHORT(rom, nv_offset + 30);
	bios->signon_msg = nv_read(rom, offset);

	init_offset = READ_SHORT(rom, nv_offset + 0x4d);

	volt_offset = READ_SHORT(rom, nv_offset + 0x98);
	parse_voltage_table(bios, rom, volt_offset);

	perf_offset = READ_SHORT(rom, nv_offset + 0x94);
	parse_nv30_performance_table(bios, rom, perf_offset);
}


static void parse_bit_structure(struct nvbios *bios, char *rom, unsigned int bit_offset)
{
	//IOLog("Parsing BIT structure\n");
	unsigned short init_offset=0;
	unsigned short perf_offset=0;
	unsigned short pll_offset=0;
	unsigned short signon_offset=0;
	unsigned short temp_offset=0;
	unsigned short volt_offset=0;
	unsigned short offset=0;

	struct bit_entry
	{
		unsigned char id[2]; /* first byte is ID, second byte sub-ID? */
		unsigned short len; /* size of data pointed to by offset */
		unsigned short offset; /* offset of data */
	} *entry;

	/* In older nvidia bioses there was some start position and at fixed positions from there offsets to various tables were stored.
	/  For Geforce6 bioses this is all different. There is still some start position (now called BIT) but offsets to tables aren't at fixed
	/  positions from the start. There's now some weird pattern which starts a few places from the start of the BIT section.
	/  This pattern seems to consist of a subset of the alphabet (all in uppercase). After each such token there is the length of the data
	/ referred to by the entry and an offset. The first entry "0x00 0x01" is probably somewhat different since the length/offset info
	/ seems to be a bit strange. The list ends with the entry "0x00 0x00"
	*/

	/* skip 'B' 'I' 'T' '\0' */
	offset = bit_offset + 4;

	/* read the entries */
	while (1)
	{
		entry = (struct bit_entry *)&rom[offset];
		if ((entry->id[0] == 0) && (entry->id[1] == 0))
			break;
		//IOLog("Got new entry: ");
		
		switch (entry->id[0])
		{
			case 'B': /* BIOS related data */
				//IOLog("BIOS");
				bios->version = nv40_bios_version_to_str(rom, entry->offset);
				break;
			case 'C': /* Configuration table; it contains at least PLL parameters */
				//IOLog("Configuration");
				pll_offset = READ_SHORT(rom, entry->offset + 8);
				parse_bit_pll_table(bios, rom, pll_offset);
				break;
			case 'I': /* Init table */
				//IOLog("Init");
				init_offset = READ_SHORT(rom, entry->offset);
				parse_bit_init_script_table(bios, rom, init_offset, entry->len);
				break;
			case 'P': /* Performance related data */
				//IOLog("Performance");
				perf_offset = READ_SHORT(rom, entry->offset);
				parse_bit_performance_table(bios, rom, perf_offset);

				temp_offset = READ_SHORT(rom, entry->offset + 0xc);
				parse_bit_temperature_table(bios, rom, temp_offset);

				/* 0x10 behind perf_offset the voltage table offset is stored */
				volt_offset = READ_SHORT(rom, entry->offset + 0x10);
				parse_voltage_table(bios, rom, volt_offset);
				break;
			case 'S':
				//IOLog("String table");
				/* table with string references of signon-message,
				BIOS version, BIOS copyright, OEM string, VESA vendor,
				VESA Product Name, and VESA Product Rev.
				table consists of offset, max-string-length pairs
				for all strings */
				//IOLog("Or panic was here?");
				signon_offset = READ_SHORT(rom, entry->offset);
				bios->signon_msg = nv_read(rom, signon_offset);
				break;
		}
		//IOLog("\n");

		offset += sizeof(struct bit_entry);
	}
	//IOLog("It has been parsed!!!\n");
}


static unsigned int locate(char *rom, const char *str, int offset)
{
	int size = strlen(str);
	int i;
	char* data;

	/* We shouldn't assume this is allways 64kB */
	for(i=offset; i<NV_PROM_SIZE; i++)
	{
		data = (char*)&rom[i];
		if(strncmp(data, str, size) == 0)
		{
			return i;
		}
	}
	return 0;
}


//#if Debug
//int main(int argc, char **argv)
//{
//	read_bios("bios.rom");
//	return 0;
//}
//
//
//#else
//#endif

/* Verify if we are dealing with a valid bios image */
int verify_bios(char *rom)
{
	/* All bioses start with this '0x55 0xAA' signature */
	if((rom[0] != 0x55) || (rom[1] != (char)0xAA))
		return 0;

	/* TODO: CRC-check, Nvidia-check */

	return 1;
}


/* Load the bios from video memory. Note it might not be cached there at all times. */
int load_bios_pramin(char *data)
{
	//int i;
	char *bios;
	uint32_t old_bar0_pramin = 0;

	/* Don't use this on unknown cards because we don't know if it needs PRAMIN fixups. */
	if(!nv_card->arch)
		return 0;

	/* On NV5x cards we need to let pramin point to the bios */
	if (nv_card->arch & NV5X)
	{
		uint32_t vbios_vram = (nv_card->PDISPLAY[0x9f04/4] & ~0xff) << 8;

		if (!vbios_vram)
			vbios_vram = (nv_card->PMC[0x1700/4] << 16) + 0xf0000;

		old_bar0_pramin = nv_card->PMC[0x1700/4];
		nv_card->PMC[0x1700/4] = (vbios_vram >> 16);
	}

	/* Copy bios data */
	bios = (char*)nv_card->PRAMIN;
	memcpy(data, bios, NV_PROM_SIZE);

	if (nv_card->arch & NV5X)
		nv_card->PMC[0x1700/4] = old_bar0_pramin;

	/* Make sure the bios is correct */
	if(verify_bios(data))
		return 1;
	else
		return 0;
}

/* Load the video bios from the ROM. Note laptops might not have a ROM which can be accessed from the GPU */
int load_bios_prom(char *data)
{
	int i;

	/* enable bios parsing; on some boards the display might turn off */
	nv_card->PMC[0x1850/4] = 0x0;

	for(i=0; i<NV_PROM_SIZE; i++)
	{
		/* On some 6600GT/6800LE boards bios there are issues with the rom.
		/  Normaly when you want to read data from lets say address X, you receive
		/  the data when it is ready. For some roms the outputs aren't "stable" yet when
		/  we want to read out the data. A workaround from Unwinder is to try to access the location
		/  several times in the hope that the outputs will become stable. In the case of instablity
		/  each fourth byte was wrong (needs to be shifted 4 to the left) and furhter there was some garbage
		/
		/  A delay of 4 extra reads helps for most 6600GT cards but for 6800Go cards atleast 5 are needed.
		*/
		data[i] = nv_card->PROM[i];
		data[i] = nv_card->PROM[i];
		data[i] = nv_card->PROM[i];
		data[i] = nv_card->PROM[i];
		data[i] = nv_card->PROM[i];
	}

	/* disable the rom; if we don't do it the screens stays black on some cards */
	nv_card->PMC[0x1850/4] = 0x1;

	/* Make sure the bios is correct */
	if(verify_bios(data))
		return 1;
	else
		return 0;
}

struct nvbios *parse_bios(char *rom)
{
	//IOLog("Parsing BIOS...\n");
	unsigned short bit_offset = 0;
	unsigned short nv_offset = 0;
	unsigned short pcir_offset = 0;
	unsigned short device_id = 0;
	struct nvbios *bios;
	//int i=0;
    
	//IOLog("Checking BIOS\n");
	/* All bioses start with this '0x55 0xAA' signature */
	if((rom[0] != 0x55) || (rom[1] != (char)0xAA))
		return NULL;
    
	//IOLog("Checking PCIR\n");
	/* Fail when the PCIR header can't be found; it is present on all PCI bioses */
	if(!(pcir_offset = locate(rom, "PCIR", 0)))
		return NULL;
    
	//IOLog("Checking vendor\n");
	/* Fail if the bios is not from an Nvidia card */
	if(READ_SHORT(rom, pcir_offset + 4) != 0x10de)
		return NULL;
    
	device_id = READ_SHORT(rom, pcir_offset + 6);
	if((get_gpu_arch(device_id) & NV4X) || (get_gpu_arch(device_id) & NV5X))
	{
		//IOLog("It's new card\n");
        /* For NV40 card the BIT structure is used instead of the BMP structure (last one doesn't exist anymore on 6600/6800le cards). */
		//IOLog("Checking BIT\n");
		if(!(bit_offset = locate(rom, "BIT", 0)))
			return NULL;
		//IOLog("Allocating memory for BIOS structure\n");
		bios = (nvbios*)IOMalloc(sizeof(nvbios));
		bios->device_id = device_id;
		parse_bit_structure(bios, rom, bit_offset);
		//IOLog("BIT structure parsed\n");
	}
	/* We are dealing with a card that only contains the BMP structure */
	else
	{
		int version;
        
		/* The main offset starts with "0xff 0x7f NV" */
		if(!(nv_offset = locate(rom, "\xff\x7fNV", 0)))
			return NULL;
        
		/* We don't support old bioses. Mainly some old tnt1 models */
		if(rom[nv_offset + 5] < 5)
			return NULL;
        
		bios = (nvbios*)IOMalloc(sizeof(nvbios));
		bios->device_id = device_id;
        
		bios->major = (char)rom[nv_offset + 5];
		bios->minor = (char)rom[nv_offset + 6];
        
		/* Go to the bios version */
		/* Not perfect for bioses containing 5 numbers */
		version = READ_INT(rom, nv_offset + 10);
		bios->version = bios_version_to_str(version);
        
		/* Use nv30_parse for all NV3X cards; for overclocking purposes the 5200 is considered
         /  a NV25 card but in this case it really is a NV3X board.
         */
		if((get_gpu_arch(device_id) & NV3X) || ((device_id & 0xff0) == 0x320))
			nv30_parse(bios, rom, nv_offset);
		else
			nv5_parse(bios, rom, nv_offset);
	}
    
#if 0
	if(bios)
	{
		int i;
		IOLog("-- VideoBios information --\n");
		IOLog("Version: %s\n", bios->version);
		IOLog("Signon message: %s\n", bios->signon_msg);
        
		for(i=0; i< bios->perf_entries; i++)
		{
			if(bios->volt_entries)
			{
				if(bios->perf_lst[i].delta)
				/* For now assume the first memory entry is the right one; should be fixed as some bioses contain various different entries */
					IOLog("Performance level %d: gpu %d(+%d)MHz/memory %dMHz/ %.2fV / %d%%\n", i, bios->perf_lst[i].nvclk, bios->perf_lst[i].delta, bios->perf_lst[i].memclk, bios->perf_lst[i].voltage, bios->perf_lst[i].fanspeed);
				else if(bios->perf_lst[i].shaderclk)
					IOLog("Performance level %d: gpu %d/shader %dMHz/memory %dMHz/ %.2fV / %d%%\n", i, bios->perf_lst[i].nvclk, bios->perf_lst[i].shaderclk, bios->perf_lst[i].memclk, bios->perf_lst[i].voltage, bios->perf_lst[i].fanspeed);
				else
					IOLog("Performance level %d: gpu %dMHz/memory %dMHz/ %.2fV / %d%%\n", i, bios->perf_lst[i].nvclk, bios->perf_lst[i].memclk, bios->perf_lst[i].voltage, bios->perf_lst[i].fanspeed);
			}
			else
				IOLog("Performance level %d: %dMHz / %dMHz / %d%%\n", i, bios->perf_lst[i].nvclk, bios->perf_lst[i].memclk, bios->perf_lst[i].fanspeed);
		}
        
		if(bios->volt_entries)
			IOLog("VID mask: %x\n", bios->volt_mask);
        
		for(i=0; i< bios->volt_entries; i++)
		{
			/* For now assume the first memory entry is the right one; should be fixed as some bioses contain various different entries */
			/* Note that voltage entries in general don't correspond to performance levels!! */
			IOLog("Voltage level %d: %.2fV, VID: %x\n", i, bios->volt_lst[i].voltage, bios->volt_lst[i].VID);
		}
		IOLog("\n");
	}
#endif
	//IOLog("Returning BIOS structure\n");    
	return bios;
}

/* This function tries to read a copy of the bios from harddrive. If that doesn't
 exist it will dump the bios and then read it. You might wonder why we don't read the bios from
 card. The reason behind that is that some bioses are slow to read (can take seconds) and second on some
 cards (atleast on my gf2mx) the screen becomes black if I enable reading of the rom.
*/
struct nvbios *read_bios(const void *data)
{
	struct nvbios *res;
	char *rom;
    
    if (!data) {
        if(!(rom = (char*)IOMalloc(NV_PROM_SIZE))) {
            //HWSensorsInfoLog("Memory allocation error");
            return NULL;
        }
        
        //if(!load_bios_pramin(rom))
        if(!load_bios_prom(rom))
        {
            //HWSensorsInfoLog("Error reading BIOS");
            IOFree(rom, NV_PROM_SIZE);
            return NULL;
        }
        
        /* Do the actual bios parsing */
        res = parse_bios(rom);
        
        /* Cleanup the mess */
        IOFree(rom, NV_PROM_SIZE);
    }
    else res = parse_bios((char*)data);	

	return res;
}