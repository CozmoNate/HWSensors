/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2008 Roderick Colenbrander
 *
 * Thanks to Erik Waling for doing Smartdimmer coding/testing. (his code isn't the one in NVClock)
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

/* This source file uses some clock calculation code from nvidia's xfree86 driver.
   To keep Nvidia happy I have added their copyright. The way they interpret it (see linux kernel riva_hw.h)
   is that you need to add the disclaimer and copyright and when that's done
   you can basicly do what you want.
*/

/***************************************************************************\ 
|*                                                                           *|
|*       Copyright 1993-2003 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.  Users and possessors of this source code are     *|
|*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
|*     use this code in individual and commercial software.                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*       Copyright 1993-2003 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
|*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
|*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
|*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
|*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
|*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
|*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
|*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
|*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
|*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
|*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
|*                                                                           *|
|*     U.S. Government  End  Users.   This source code  is a "commercial     *|
|*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
|*     consisting  of "commercial  computer  software"  and  "commercial     *|
|*     computer  software  documentation,"  as such  terms  are  used in     *|
|*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
|*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
|*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
|*     all U.S. Government End Users  acquire the source code  with only     *|
|*     those rights set forth herein.                                        *|
|*                                                                           *|
\***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "backend.h"

/*
/ The original NV40 gpu was used as the base for 6800LE/6800NU/6800GT/6800Ultra
/ GPUs. The difference between all these models lie in the amount of enabled
/ pixel/vertex pipelines and clockspeeds. For instance the 6800LE ships with 8
/ pixel pipelines while the 6800GT ships with 16 of those. Right after production
/ all GPUs are tested, if all pipelines work and they run at high clocks they
/ are called Ultra or if pipes are broken they are called 6800NU(12p) or 6800LE(8p).
/ Further in some cases 'good' GPUs can be rebranded too if there's a shortage of
/ 6800NU/6800LE GPUs. The key to this rebranding is register 0x1540 which contains
/ the enabled pixel/vertex pipelines. Depending on the GPU architecture a bit can
/ correspond to a single vertex shader or to a block containing two or four
/ pixel pipelines.
/
/ We now define some words coming from Rivatuner as people are familiar with those.
/ A 'good' GPU for which pipelines are disabled just to get enough lowend models
/ is said to contain 'Software masked units'. In this case the videobios initializes
/ 0x1540 with a value that locks out some units.
/ GPUs which didn't pass the hardware quality testing contain 'Hardware masked units'.
/ In this case the bios initializes 0x1540 with a value that enables all pipelines.
/ A certain (read-only) register (0xc010) contains a mask of units to disable by default.
/ The bios loads this value into another register (0xc020) at startup. The value from
/ 0xc020 is then used by the drivers to disable units in 0x1540. For example by clearing this
/ register before the drivers are loaded, you can prevent masks from being disabled.

/ 1540 units_cfg (rw) second byte contains vertex configuration and first byte pipeline
/ c010 default_mask (r) pixel pipelines start at +22, while vertex start at +16 (is this correct for all cards?)
/ c020 active_mask (rw)
/ c024/c028/c02c are being set to 0, why? (they correspond to c014/c018/c01c)
/
/ Below are supported pipeline configurations on various types of cards. Not sure
/ if everything is fully correct though:
/ - NV40 0x3f0f 6800 cards
/ - NV41 0x1f07 6800 pci-e (is this correct?)
/ - NV43 0x0703 6200/6600 cards
/ - NV44 0x0703 6200(Go)/Turbocache cards
/ - NV46 0x0703 7300 (is this correct?)
/ - NV47/NV49 0xff3f 7800/7900 cards
/ - NV4B 0x1f0f 7600
*/

static int nv40_get_default_mask(char *pmask, char *vmask)
{
	int mask;
	switch(nv_card->arch)
	{
		case NV40:
			mask = 0x3f0f;
			break;
		case NV41:
			mask = 0x1f07;
			break;
		case NV43:
		case NV44:
		case NV46:
		case C51:
			mask = 0x703;
			break;
		case NV47:
		case NV49:
			mask = 0xff3f;
			break;
		case NV4B:
			mask = 0x1f0b;
			break;
	}

	if(pmask)
		*pmask = mask & 0xff;
	if(vmask)
		*vmask = (mask >> 8) & 0xff;

	return mask;
}

/* Try to locate hardware maskes units. On success we return 1 and pmask/vmask
/  contain the masked units. When no hw masked units are found we return 0.
*/
static int nv40_get_hw_masked_units(char *pmask, char *vmask)
{
	unsigned int mask = nv_card->PMC[0xc010/4]; /* Or should we show the currently locked pipes? */
	unsigned int masked_units;

	/* On 6200 cards for some reason 0xc010 can be empty while there are locked pipes, 0xc020 is then used instead.
	/  For now we use 0xc020 if that's the case. Note that during unlocking 0xc020 is cleared and that we then lose this locking info.
	*/
	if(mask == 0)
		mask = nv_card->PMC[0xc020/4];
	/* For now we show units that are hw masked by default, not the currently masked units; the cli code wants to have this info
	/  Unfortunately bios dumping isn't possible on various mobile 6200go cards, so as a fallback use the currently masked pipes
	/  in favor of a segfault ;)
	*/
	/* What to do with NV47 which has 8 vertex units? */
	masked_units = (((mask & 0x3f0000) >> 8) | (mask >> 22)) & nv40_get_default_mask(0, 0);

	if(masked_units != 0)
	{
		*pmask = masked_units & 0xff; /* pixel */
		*vmask = (masked_units >> 8) & 0xff; /* vertex */
		return 1;
	}

	return 0;
}

/* Try to locate software maskes units. On success we return 1 and pmask/vmask
/  contain the masked units. When no sw masked units are found we return 0.
*/
static int nv40_get_sw_masked_units(char *pmask, char *vmask)
{
	unsigned int mask = nv40_get_default_mask(0, 0);
	unsigned int pipe_cfg;

	pipe_cfg = nv_card->PMC[0x1540/4] & nv40_get_default_mask(0, 0);

	if(!pipe_cfg)
		return 0;

	/* Check if the card contains sw masked units by comparing
	/  the default pipe_cfg register value with the most optimal
	/  register value for the type of card. If they differ we have
	/  sw masked units. The check below first does a AND-mask to filter
	/  out bits which aren't needed.
	*/
	if((pipe_cfg & 0xffff) != mask)
	{
		/* First note the bits that are different
		/  E.g. pipe_cfg = 0x701, while mask = 0x703
		/  0x701 ^ 0x703 = 0x002
		*/
		pipe_cfg = (pipe_cfg ^ mask) & mask;
		*pmask = pipe_cfg & 0xff;
		*vmask = (pipe_cfg >> 8) & 0xff;
		return 1;
	}
	return 0;
}

/* Receive the number of enabled pixel pipelines and also
/  store a mask with active pipelines. Further store the total
/  number of pixel units per pipeline in total.
*/
static int nv40_get_pixel_pipelines(char *mask, int *total)
{
	unsigned char pipe_cfg = nv_card->PMC[0x1540/4] & 0xff;
	int i, pipelines=0;

	/* The number of enabled pixel pipelines is stored in the first 4 (or more?) bits.
	/  In case of 6800 hardware a single bit corresponds to 4 pipelines and on NV44/NV46
	/  hardware a bit corresponds to 2 pipelines
	*/
	for(i=0; i<8; i++)
		if((pipe_cfg >> i) & 0x1)
			pipelines++;

	*mask = pipe_cfg;

	/* NV44/NV46 use 2 pixel units per pipeline */
	if(nv_card->arch & (NV44 | NV46))
		*total = 2;
	else
		*total = 4;
	
	return pipelines;
}

/* Receive the number of enabled vertex pipelines and also
/  store a mask with active pipelines.
*/
static int nv40_get_vertex_pipelines(char *mask)
{
	unsigned char pipe_cfg = (nv_card->PMC[0x1540/4]  >> 8) & 0xff;
	int i, pipelines=0;

	/* The number of enabled vertex pipelines is stored in the second byte.
	/  A a single bit corresponds to 1 vertex pipeline.
	*/
	for(i=0; i<8; i++)
		if((pipe_cfg >> i) & 0x1)
			pipelines++;

	*mask = pipe_cfg;

	return pipelines;
}

static void nv40_set_pixel_pipelines(unsigned char mask)
{
	int pipe_cfg = nv_card->PMC[0x1540/4];

	/* Why do 0xc024/0xc028/0xc02c need to be reset? What do they contain? */
	nv_card->PMC[0xc020/4] = nv_card->PMC[0xc024/4] = nv_card->PMC[0xc028/4] = nv_card->PMC[0xc02c/4] = 0;

	nv_card->PMC[0x1540/4] = ~(~pipe_cfg | 0xff) | mask;
}

static void nv40_set_vertex_pipelines(unsigned char mask)
{
	int pipe_cfg = nv_card->PMC[0x1540/4];

	/* Why do 0xc024/0xc028/0xc02c need to be reset? What do they contain? */
	nv_card->PMC[0xc020/4] = nv_card->PMC[0xc024/4] = nv_card->PMC[0xc028/4] = nv_card->PMC[0xc02c/4] = 0;

	nv_card->PMC[0x1540/4] = ~(~pipe_cfg | 0xff00) | (mask<<8);
}

/* Fanspeed code for Geforce6800 hardware */
static float nv40_get_fanspeed()
{
	/* Bit 30-16 of register 0x10f0 control the voltage for the pwm signal generator
	/  which is connected to the fan. By changing the value in the register the duty cycle can be controlled
	/  so that the fan turns slower or faster. Bit 14-0 of 0x10f0 contain the pwm division
	/  ratio which decides the smallest fanspeed adjustment step.
	/  The value stored in the registers needs to be inverted, so a value of 10% means 90% and so on.
	*/
	int pwm_divider = nv_card->PMC[0x10f0/4] & 0x7fff;
	float fanspeed = (float)(pwm_divider - ((nv_card->PMC[0x10f0/4] >> 16) & 0x7fff)) * 100.0/(float)pwm_divider;
	return fanspeed;
}

static void nv40_set_fanspeed(float speed)
{
	int value;
	int pwm_divider = nv_card->PMC[0x10f0/4] & 0x7fff;

	/* For safety reasons we should never disable the fan by not putting it below 10%; further negative values don't exist ;)  */
	if(speed < 10 || speed > 100)
		return;

	value = 0x80000000 + ((((int)(100 - speed) * pwm_divider/100) & 0x7fff)<<16) + pwm_divider;
	nv_card->PMC[0x10f0/4] = value;
}

/* Fanspeed code for Geforce6600 hardware (does this work for 6200 cards too??)*/
static float nv43_get_fanspeed()
{
	/* The first 12 or more bits of register 0x15f4 control the voltage for the pwm signal generator in case
	/  of Geforce 6200/6600(GT)/7600/7800GS hardware. By changing the value in the register the duty cycle of the pwm signal
	/  can be controlled so that the fan turns slower or faster. The first part of register 0x15f8 contains the pwm division ratio.
	/  The value stored in the registers needs to be inverted, so a value of 10% means 90% and so on. (pwm_divider means off, 0 means on)
	*/
	int pwm_divider = nv_card->PMC[0x15f8/4] & 0x3fff;
	float fanspeed = (pwm_divider - (nv_card->PMC[0x15f4/4] & 0x3fff)) * 100.0/(float)pwm_divider;
	return fanspeed;
}

static void nv43_set_fanspeed(float speed)
{
	int value;
	int pwm_divider = nv_card->PMC[0x15f8/4] & 0x3fff;

	/* For safety reasons we should never disable the fan by not putting it below 10%; further negative values don't exist ;) */
	if(speed < 10 || speed > 100)
		return;

	value = 0x80000000 + (int)((100 - speed) * pwm_divider/100);
	nv_card->PMC[0x15f4/4] = value;
}

/* There's an internal temperature sensor on NV43 hardware and higher
/  Note that the sensor variable which is passed to this function is bogus
/  it is only there to share nv_card->get_gpu_temp between I2C and low-level.
*/
static int nv43_get_gpu_temp(void *sensor)
{
	int temp;
	int correction=0;
	float offset;
	float slope;

	/* For now duplicate the temperature offset code here. It is needed for Mobile users in most cases the bios can't be read on those GPUs. */
	if(!nv_card->bios)
	{
		switch(nv_card->arch)
		{
			case NV43:
				offset = 32060.0/1000.0;
				slope = 792.0/1000.0;
				break;
			case NV44:
			case NV47:
				offset = 27839.0/1000.0;
				slope = 700.0/1000.0;
				break;
			case NV46: /* are these really the default ones? they come from a 7300GS bios */
				offset = -24775.0/100.0;
				slope = 467.0/10000.0;
				break;
			case NV49: /* are these really the default ones? they come from a 7900GT/GTX bioses */
				offset = -25051.0/100.0;
				slope = 458.0/10000.0;
				break;
			case NV4B: /* are these really the default ones? they come from a 7600GT bios */
				offset = -24088.0/100.0;
				slope = 442.0/10000.0;
				break;
		}
	}
	else
	{
		/* The output value of the sensor needs to be 'calibrated' in order to get the correct temperature. These
		/  values are stored in the video bios and are different for each type of gpu. The value needs to be multiplied
		/  with a certain 'slope' and further the sensor has an offset and another correction constant.
		*/
		offset = (float)nv_card->bios->sensor_cfg.diode_offset_mult / (float)nv_card->bios->sensor_cfg.diode_offset_div;
		slope = (float)nv_card->bios->sensor_cfg.slope_mult / (float)nv_card->bios->sensor_cfg.slope_div;
		correction = nv_card->bios->sensor_cfg.temp_correction;
	}

	/* Assume that the sensor is disabled when the temperature part (without offset) is 0 */
	if((nv_card->PMC[0x15b4/4] & 0xfff) == 0)
	{
		/* Initialize the sensor, for now program a threshold value of 120C.
		*/
		int max_temp = (int)(((float)120 - offset - correction) / slope);

		/* 7300/7600/7900 cards need bit31 to be set while older cards need a different bit */
		if(nv_card->arch & (NV46 | NV49 | NV4B))
			nv_card->PMC[0x15b0/4] = 0x80000000 | max_temp;
		else
			nv_card->PMC[0x15b0/4] = 0x10000000 | max_temp;
		usleep(500);
	}

	/* In case of Geforce 7300/7600/7900 cards more than one byte is used for the temperature */
	if(nv_card->arch & (NV46 | NV49 | NV4B))
		temp = nv_card->PMC[0x15b4/4] & 0x1fff;
	else
		temp = nv_card->PMC[0x15b4/4] & 0xff;

	if(nv_card->debug)
	{
		printf("NV_15B4 (0x15B4): %08x\n", nv_card->PMC[0x15b4/4]);
		printf("slope=%f, offset=%f, correction=%d\n", slope, offset, correction);
	}

	return (int)(temp * slope + offset) + correction;
}

/* Get current backpanel brightness level on laptops */
static int nv44_mobile_get_smartdimmer()
{
	/* Convert level to a value between 1 and 100 */
	return 5*(((nv_card->PMC[0x15f0/4] >> 16) & 0x1f) - 1);
}

/* Adjust backpanel brightness on laptops */
static void nv44_mobile_set_smartdimmer(int level)
{
	if(level < 15 || level > 100)
		return;

	/* Convert the level to correct Smartdimmer values; on Windows a value between 4 and 21 works fine although 0-31 should work.
	/  The code below creates a value between 4 and 21;
	*/
	level = level/5 + 1;

	/* Modify the smartdimmer part but keep the rest of the register the same */
	nv_card->PMC[0x15f0/4] = (level << 16) | (nv_card->PMC[0x15f0/4] & 0xffe0ffff);
}

static int CalcSpeed_nv40(int base_freq, int m1, int m2, int n1, int n2, int p)
{
	return (int)((float)(n1*n2)/(m1*m2) * base_freq) >> p;
}

float GetClock_nv40(int base_freq, unsigned int pll, unsigned int pll2)
{
	int m1, m2, n1, n2, p;

	/* mpll at 0x4020 and 0x4024; nvpll at 0x4000 and 0x4004 */
	p = (pll >> 16) & 0x03;
	m1 = pll2 & 0xFF;
	n1 = (pll2 >> 8) & 0xFF;
	
	/* Bit 8 of the first pll register can be used to disable the second set of dividers/multipliers. */
	if(pll & 0x100)
	{
		m2 = 1;
		n2 = 1;
	}
	/* NV46/NV49/NV4B cards seem to use a different calculation; I'm not sure how it works yet, so for now check the architecture. Further it looks like bit 15 can be used to identify it but I'm not sure yet.
	*/
	else if(pll & 0x1000)
	{
		m2 = 1;
		n2 = 1;
	}
	else
	{
		m2 = (pll2 >> 16) & 0xFF;
		n2 = (pll2 >> 24) & 0xFF;
	}

	if(nv_card->debug)
		printf("m1=%d m2=%d n1=%d n2=%d p=%d\n", m1, m2, n1, n2, p);

	return (float)CalcSpeed_nv40(base_freq, m1, m2, n1, n2, p)/1000;
}

const int pll_entries=2;
/* TODO: add proper architecture specific defaults */
const struct pll pll_lst[2] = { 
	{0x0000, 7, 0, {3000, 25000, 100000, 405000, 1, 255, 1, 255}, {35000, 100000, 400000, 1000000, 1, 31, 1, 31}},
	{0x4020, 7, 0, {3000, 25000, 100000, 405000, 1, 255, 1, 255}, {35000, 100000, 600000, 1400000, 1, 31, 1, 31}},
};

static const struct pll* GetPllLimits(unsigned int reg)
{
	int i;
	if(nv_card->bios && nv_card->bios->pll_entries)
	{
		for(i=0; i<nv_card->bios->pll_entries; i++)
		{
			if(nv_card->bios->pll_lst[i].reg == reg)
				return &nv_card->bios->pll_lst[i];
		}
		/* Return the default limits */
		return &nv_card->bios->pll_lst[0];
	}
	else
	{
		for(i=0; i<pll_entries; i++)
		{
			if(pll_lst[i].reg == reg)
				return &pll_lst[i];
		}
		/* Return the default limits */
		return &pll_lst[0];
	}
}

static void ClockSelectSingleVCO_nv40(int clockIn, const struct pll *pll_limits, int *bestM, int *bestN, int *bestP)
{
	int clock, M, N, P;
	int delta, bestDelta, minM, maxM, minN, maxN, maxP;
	int minVCOInputFreq, minVCOFreq, maxVCOFreq;
	int refClk = 27000;
	int VCOFreq;
	*bestM=*bestN=*bestP=0;
	bestDelta = clockIn;

	minVCOInputFreq = pll_limits->VCO1.minInputFreq;
	minVCOFreq = pll_limits->VCO1.minFreq;
	maxVCOFreq = pll_limits->VCO1.maxFreq;
	minM = pll_limits->VCO1.minM;
	maxM = pll_limits->VCO1.maxM;
	minN = pll_limits->VCO1.minN;
	maxN = pll_limits->VCO1.maxN;
	maxP = 6;

	/* The optimal frequency for the PLL to work at is somewhere in the center of its range.
	/  Choose a post divider in such a way to achieve this.
	/  The G8x nv driver does something similar but they they derive a minP and maxP. That
	/  doesn't seem required as you get so many matching clocks that you don't enter a second
	/  iteration for P. (The minP / maxP values in the nv driver only differ at most 1, so it is for
	/  some rare corner cases.
	*/
	for(P=0, VCOFreq=maxVCOFreq/2; clockIn<=VCOFreq && P <= maxP; P++)
	{
		VCOFreq /= 2;
	}

	/* Calculate the m and n values. There are a lot of values which give the same speed;
	/  We choose the speed for which the difference with the request speed is as small as possible.
	*/
	for(M=minM; M<=maxM; M++)
	{
		/* The VCO has a minimum input frequency */
		if((refClk/M) < minVCOInputFreq)
			break;

		for(N=minN; N<=maxN; N++)
		{
			/* Calculate the frequency generated by VCO1 */
			clock = (int)(refClk * N / (float)M);

			/* Verify if the clock lies within the output limits of VCO1 */
			if(clock < minVCOFreq)
				continue;
 			else if (clock > maxVCOFreq) /* It is no use to continue as the clock will only become higher */
				break;

			clock >>= P;
			delta = abs((int)(clockIn - clock));
			/* When the difference is 0 or less than .5% accept the speed */
			if(((delta == 0) || ((float)delta/(float)clockIn <= 0.005)))
			{
				*bestM = M;
				*bestN = N;
				*bestP = P;
				return;
			}

			/* When the new difference is smaller than the old one, use this one */
			if(delta < bestDelta)
			{
				bestDelta = delta;
				*bestM = M;
				*bestN = N;
				*bestP = P;
			}
		}
	}
}

static void ClockSelectDoubleVCO_nv40(int clockIn, const struct pll *pll_limits, int *bestM, int *bestM2, int *bestN, int *bestN2, int *bestP)
{
	int clock1, clock2, M, M2, N, N2, P;
	int delta, bestDelta, minM, minM2, maxM, maxM2, minN, minN2, maxN, maxN2, maxP;
	int minVCOInputFreq, minVCO2InputFreq, maxVCO2InputFreq, minVCOFreq, minVCO2Freq, maxVCOFreq, maxVCO2Freq;
	int maxClock, VCO2Freq;
	int refClk = 27000;
	bestDelta = clockIn;
	*bestM=*bestM2=*bestN=*bestN2=*bestP=0;

	minVCOInputFreq = pll_limits->VCO1.minInputFreq;
	minVCOFreq = pll_limits->VCO1.minFreq;
	maxVCOFreq = pll_limits->VCO1.maxFreq;
	minM = pll_limits->VCO1.minM;
	maxM = pll_limits->VCO1.maxM;
	minN = pll_limits->VCO1.minN;
	maxN = pll_limits->VCO1.maxN;

	minVCO2InputFreq = pll_limits->VCO2.minInputFreq;
	maxVCO2InputFreq = pll_limits->VCO2.maxInputFreq;
	minVCO2Freq = pll_limits->VCO2.minFreq;
	maxVCO2Freq = pll_limits->VCO2.maxFreq;
	minM2 = pll_limits->VCO2.minM;
	maxM2 = pll_limits->VCO2.maxM;
	minN2 = pll_limits->VCO2.minN;
	maxN2 = pll_limits->VCO2.maxN;
	maxP = 6; /* This should be somewhere in the bios too */

	maxClock = maxVCO2Freq;	
	/* If the requested clock is behind the bios limits, try it anyway */
	if(clockIn > maxVCO2Freq)
		maxClock = clockIn + clockIn/200; /* Add a .5% margin */

	/* The optimal frequency for the PLL to work at is somewhere in the center of its range.
	/  Choose a post divider in such a way to achieve this.
	/  The G8x nv driver does something similar but they they derive a minP and maxP. That
	/  doesn't seem required as you get so many matching clocks that you don't enter a second
	/  iteration for P. (The minP / maxP values in the nv driver only differ at most 1, so it is for
	/  some rare corner cases.
	*/
	for(P=0, VCO2Freq=maxClock/2; clockIn<=VCO2Freq && P <= maxP; P++)
	{
		VCO2Freq /= 2;
	}

	/* The PLLs on Geforce6/7 hardware can operate in a single stage made with only 1 VCO
	/  and a cascade mode of two VCOs. This second mode is in general used for relatively high
	/  frequencies. The loop below calculates the divider and multiplier ratios for the cascade
	/  mode. The code takes into account limits defined in the video bios.
	*/
	for(M=minM; M<=maxM; M++)
	{
		/* The VCO has a minimum input frequency */
		if((refClk/M) < minVCOInputFreq)
			break;

		for(N=minN; N<=maxN; N++)
		{
			/* Calculate the frequency generated by VCO1 */
			clock1 = (int)(refClk * N / (float)M);
			/* Verify if the clock lies within the output limits of VCO1 */
			if( (clock1 < minVCOFreq) )
				continue;
			else if(clock1 > maxVCOFreq) /* For future N, the clock will only increase so stop; xorg nv continues but that is useless */
				break;

			for(M2=minM2; M2<=maxM2; M2++)
			{
				/* The clock fed to the second VCO needs to lie within a certain input range */
				if(clock1 / M2 < minVCO2InputFreq)
					break;
				else if(clock1 / M2 > maxVCO2InputFreq)
					continue;

				N2 = (int)((float)((clockIn << P) * M * M2) / (float)(refClk * N)+.5);
				if( (N2 < minN2) || (N2 > maxN2) )
					continue;

				/* The clock before being fed to the post-divider needs to lie within a certain range.
				/  Further there are some limits on N2/M2.
				*/
				clock2 = (int)((float)(N*N2)/(M*M2) * refClk);
				if( (clock2 < minVCO2Freq) || (clock2 > maxClock))// || ((N2 / M2) < 4) || ((N2 / M2) > 10) )
					continue;

				/* The post-divider delays the 'high' clock to create a low clock if requested.
				/  This post-divider exists because the VCOs can only generate frequencies within
				/  a limited frequency range. This range has been tuned to lie around half of its max
				/  input frequency. It tries to calculate all clocks (including lower ones) around this
				/  'center' frequency.
				*/
				clock2 >>= P;
				delta = abs((int)(clockIn - clock2));

				/* When the difference is 0 or less than .5% accept the speed */
				if(((delta == 0) || ((float)delta/(float)clockIn <= 0.005)))
				{
					*bestM = M;
					*bestM2 = M2;
					*bestN = N;
					*bestN2 = N2;
					*bestP = P;
					return;
				}
	
				/* When the new difference is smaller than the old one, use this one */
				if(delta < bestDelta)
				{
					bestDelta = delta;
					*bestM = M;
					*bestM2 = M2;
					*bestN = N;
					*bestN2 = N2;
					*bestP = P;
				}
			}
		}
	}
}

static void ClockSelect_nv40(int clockIn, unsigned int reg, unsigned int *pllOut, unsigned int *pllBOut)
{
	int PLL=0, PLL2=0;
	int bestM=0, bestM2=1, bestN=0, bestN2=1, bestP=0;
	const struct pll *pll_limits = GetPllLimits(reg);
	int pllIn = nv_card->PMC[reg/4];

	printf("Warning using experimental NV4x lowlevel clock adjustment, if you encounter strange issues, issue a bugreport.\n");

	/* Use a single VCO if the clock isn't high enough to require a second.
	/  This is what apparently the Nvidia drivers are doing. For instance on my 7600GS
	/  the max frequency for the first VCO in case of the GPU clock is 300MHz. Indeed below
	/  300MHz it moves to a single VCO.
	/  We could likely also check if the limits of the second divider and multiplier are 1.  */
	if(clockIn < pll_limits->VCO1.maxFreq)
	{
		ClockSelectSingleVCO_nv40(clockIn, pll_limits, &bestM, &bestN, &bestP);

		/* Bit31 enables the first VCO */
		PLL = 0x80000000;

		/* Select only a single VCO; bit12 is apparently used when there is a single VCO (G7x)
		/  while bit8 is used when there are two VCOs, so it is apparently some disable bit.
		/
		/  On some cards at least on G7x ones, a single VCO is used for the memory
		/  In this case the min and max reference dividers (M) are equal.
		*/
		if(pll_limits->VCO2.minM == pll_limits->VCO2.maxM)
			PLL |= 0x1000;
		else
			PLL |= 0x100;

		/* Set the reference divider (M) and the feedback divider (N) */
		PLL2 = (bestN<<8) | bestM;
	}
	else
	{
		ClockSelectDoubleVCO_nv40(clockIn, pll_limits, &bestM, &bestM2, &bestN, &bestN2, &bestP);

		/* Bit31 enables the first VCO, bit30 the second */
		PLL = 0xc0000000;
		/* Set the reference dividers (M, M2) and the feedback dividers (N, N2) */
		PLL2 = (bestN2<<24) | (bestM2<<16) | (bestN<<8) | bestM;
	}

	/* Set the post divider */
	PLL |= (bestP<<16);

	if(reg == 0x4020) /* MPLL */
	{
		unsigned int default_pll = nv_card->bios ? nv_card->bios->mpll : 0;

		/* This data comes from the init script tables of bios of the respective cards */
		if(!default_pll)
		{
			switch(nv_card->arch)
			{
				case NV40:
				case NV41: /* I'm not sure if this is correct */
				case NV43:
				case NV44:
					default_pll = 0x2000001c;
					break;
				case NV46:
					default_pll = 0x20000000;
					break;
				case NV47:
					default_pll = 0x2400001c;
					break;
				case NV49:
				case NV4B:
					default_pll = 0x24800000;
					break;
				default:
					printf("Unknown default pll for %#x\n", nv_card->arch);
			}
		}

		/* According to a vbtracetool log, this is done. Why a second post divider for the MPLL? */
		PLL |= (bestP + pll_limits->var1e) << 20;
		/* Or with the 'empty' PLL but make sure that it doesn't adjust the post dividers or the enable bits */
		PLL |= (default_pll & 0x3f88ffff);
	}
	else /* NVPLL */
	{
		unsigned int default_pll = nv_card->bios ? nv_card->bios->nvpll : 0;
		if(!default_pll)
		{
			switch(nv_card->arch)
			{
				case NV40:
				case NV41: /* I'm not sure if this is correct */
				case NV43:
					default_pll = 0x0000001c;
					break;
				case NV44:
					default_pll = 0xc000001c;
					break;
				case NV46:
					default_pll = 0xc0000000;
					break;
				case NV47:
					default_pll = 0x0001001f;
					break;
				case NV49:
				case NV4B:
					default_pll = 0x00010000;
					break;
				case C51:
				default:
					printf("Unknown default pll for %#x\n", nv_card->arch);
					default_pll = 0xc0000000; /* this should only get reached for C51 */
			}
		}

		/* Or with the 'empty' PLL but make sure that it doesn't adjust the post dividers or the enable bits */
		PLL |= (default_pll & 0x3f88ffff);
	}

	/* TODO: at some stage we should perhaps also play nice with 0xc040.
	/ Before programming a PLL, we should disable it in there. Then
	/ we should put the PLL in program mode (?) by setting bit28. After that
	/ we should program the PLLs (first the dividers/multipliers and then the config),
	/ then re-enable the PLL in 0xc040 and then unset bit28 of the pll config register.
	/ At least this is what happens in a vbtracetool log.
	*/
	*pllOut = PLL;
	*pllBOut = PLL2;

	if(nv_card->debug)
	{
		printf("register=%#x, clockIn=%d, calculated=%d\n", reg, clockIn, CalcSpeed_nv40(27000, bestM, bestM2, bestN, bestN2, bestP));
		printf("PLL=%#08x, PLL2=%08x\n", *pllOut, *pllBOut);
	}
}

static float nv40_get_gpu_speed()
{
	int pll = nv_card->PMC[0x4000/4];
	int pll2 = nv_card->PMC[0x4004/4];
	if(nv_card->debug == 1)
	{
		printf("NVPLL_COEFF=%08x\n", pll);
		printf("NVPLL2_COEFF=%08x\n", pll2);
	}

	return (float)GetClock_nv40(nv_card->base_freq, pll, pll2);
}

static void nv40_set_gpu_speed(unsigned int nvclk)
{
	unsigned int PLL=0, PLL2=0;
	nvclk *= 1000;

	ClockSelect_nv40(nvclk, 0x4000, &PLL, &PLL2);

	/* When no speed is found, don't change the PLL */
	/* The algorithm doesn't allow too low speeds */
	if(PLL)
	{
		if(nv_card->debug)
		{
			printf("NVPLL_COEFF: %08x\n", PLL);
			printf("NVPLL2_COEFF: %08x\n", PLL2);
		}

		nv_card->PMC[0x4000/4] = PLL;
		nv_card->PMC[0x4004/4] = PLL2;
	}
}

static float nv40_get_memory_speed()
{
	int pll = nv_card->PMC[0x4020/4];
	int pll2 = nv_card->PMC[0x4024/4];
	if(nv_card->debug == 1)
	{
		printf("MPLL_COEFF=%08x\n", pll);
		printf("MPLL2_COEFF=%08x\n", pll2);
	}

	return (float)GetClock_nv40(nv_card->base_freq, pll, pll2);
}

static void nv40_set_memory_speed(unsigned int memclk)
{
	unsigned int PLL=0, PLL2=0;
	memclk *= 1000;

	ClockSelect_nv40(memclk, 0x4020, &PLL, &PLL2);

	/* When no speed is found, don't change the PLL */
	/* The algorithm doesn't allow too low speeds */
	if(PLL)
	{
		if(nv_card->debug)
		{
			printf("MPLL_COEFF: %08x\n", PLL);
			printf("MPLL2_COEFF: %08x\n", PLL2);
		}

		/* It seems that different NV4X GPUs contain multiple memory clocks.
		/  A 6800 card has 4 of them, a 6600GT 2 of them and a NV44 (6200) 1.
		/  Very likely this is related to the width of the memory bus, which
		/  is 256bit on the 6800, 128bit on the 6600GT (NV43) and 64bit on the NV44.
		/
		/  The code below handles the setting of the extra clockspeeds.
		*/
		switch(nv_card->arch)
		{
			case NV40:
			case NV41:
			case NV47:
				nv_card->PMC[0x402c/4] = PLL;
				nv_card->PMC[0x4030/4] = PLL2;
				nv_card->PMC[0x4044/4] = PLL;
				nv_card->PMC[0x4048/4] = PLL2;
			case NV43:
			case NV49:
			case NV4B:
				nv_card->PMC[0x4038/4] = PLL;
				nv_card->PMC[0x403c/4] = PLL2;
			case NV44:
			case NV46:
				nv_card->PMC[0x4020/4] = PLL;
				nv_card->PMC[0x4024/4] = PLL2;
		}
	}
}

static void nv40_reset_gpu_speed()
{
	/* Set the gpu speed */
	nv_card->PMC[0x4000/4] = nv_card->nvpll;
	nv_card->PMC[0x4004/4] = nv_card->nvpll2;
}

static void nv40_reset_memory_speed()
{
	/* Set the memory speed */
	nv_card->PMC[0x4024/4] = nv_card->mpll2;

	switch(nv_card->arch)
	{
		case NV40:
		case NV41:
		case NV47:
			nv_card->PMC[0x402c/4] = nv_card->mpll;
			nv_card->PMC[0x4030/4] = nv_card->mpll2;
			nv_card->PMC[0x4044/4] = nv_card->mpll;
			nv_card->PMC[0x4048/4] = nv_card->mpll2;
		case NV43:
		case NV49:
		case NV4B:
			nv_card->PMC[0x4038/4] = nv_card->mpll;
			nv_card->PMC[0x403c/4] = nv_card->mpll2;
		case NV44:
		case NV46:
			nv_card->PMC[0x4020/4] = nv_card->mpll;
			nv_card->PMC[0x4024/4] = nv_card->mpll2;
	}
}

static void nv40_set_state(int state)
{
#ifdef HAVE_NVCONTROL
	if(state & (STATE_2D | STATE_3D))
	{
		nv_card->state = state;
		nv_card->get_gpu_speed = nvcontrol_get_gpu_speed;
		nv_card->get_memory_speed = nvcontrol_get_memory_speed;
		nv_card->set_gpu_speed = nvcontrol_set_gpu_speed;
		nv_card->set_memory_speed = nvcontrol_set_memory_speed;
		nv_card->reset_gpu_speed = nvcontrol_reset_gpu_speed;
		nv_card->reset_memory_speed = nvcontrol_reset_memory_speed;	
	}
	else
#endif
	{
		nv_card->state = STATE_LOWLEVEL;
		nv_card->get_gpu_speed = nv40_get_gpu_speed;
		nv_card->get_memory_speed = nv40_get_memory_speed;
		nv_card->set_memory_speed = nv40_set_memory_speed;
		nv_card->set_gpu_speed = nv40_set_gpu_speed;
		nv_card->reset_gpu_speed = nv40_reset_gpu_speed;
		nv_card->reset_memory_speed = nv40_reset_memory_speed;
	}
}

void nv40_init(void)
{
	nv_card->base_freq = 27000;

	nv_card->set_state = nv40_set_state;
	nv_card->set_state(STATE_LOWLEVEL); /* Set the clock function pointers */	
	
	nv_card->get_default_mask = nv40_get_default_mask;
	nv_card->get_hw_masked_units = nv40_get_hw_masked_units;
	nv_card->get_sw_masked_units = nv40_get_sw_masked_units;
	nv_card->get_pixel_pipelines = nv40_get_pixel_pipelines;
	nv_card->get_vertex_pipelines = nv40_get_vertex_pipelines;

	/* Register I2C busses for hardware monitoring purposes */
	if(nv_card->busses[0] == NULL)
	{
		nv_card->num_busses = 3;
		nv_card->busses[0] = NV_I2CCreateBusPtr("BUS0", 0x3e); /* available on riva128 and higher */
		nv_card->busses[1] = NV_I2CCreateBusPtr("BUS1", 0x36); /* available on rivatnt hardware and  higher */
		nv_card->busses[2] = NV_I2CCreateBusPtr("BUS2", 0x50);  /* available on geforce4mx/4ti/fx/6/7 */

		i2c_sensor_init();
	}

	/* For now enable modding on NV40 cards and NV43 revisions prior to A4; other cards are locked */
	if((nv_card->arch & NV40) || ((nv_card->arch & NV43) && (nv_card->get_gpu_revision() < 0xA4)))
	{
		nv_card->caps |= PIPELINE_MODDING;
		nv_card->set_pixel_pipelines = nv40_set_pixel_pipelines;
		nv_card->set_vertex_pipelines = nv40_set_vertex_pipelines;
	}

	/* If the smartdimmer register contains a value (default 21) then smartdimmer is supported on the laptop; This should work on various 6200Go/7600Go cards */
	if((nv_card->PMC[0x15f0/4] & 0xff) && nv_card->gpu == MOBILE)
	{
		nv_card->caps |= SMARTDIMMER;
		nv_card->get_smartdimmer = nv44_mobile_get_smartdimmer;
		nv_card->set_smartdimmer = nv44_mobile_set_smartdimmer;
	}
	
	/* Temperature monitoring; all cards after the NV40 feature an internal temperature sensor.
	/  Only it is disabled on most 6200/6600(GT) cards but we can re-enable it ;)
	*/
	if((nv_card->arch & (NV43 | NV44 | NV46 | NV47 | NV49 | NV4B)) && !(nv_card->caps & GPU_TEMP_MONITORING))
	{
		nv_card->caps |= GPU_TEMP_MONITORING;
		nv_card->sensor_name = (char*)strdup("GPU Internal Sensor");
		nv_card->get_gpu_temp = (int(*)(I2CDevPtr))nv43_get_gpu_temp;
	}

	/* Fanspeed monitoring; bit 31 is an indication if fanspeed monitoring is available
	/  Note this bit isn't very reliable as it is set on cards with advanced sensors too.
	/  Should the NV44 use the NV43 codepath?
	*/
	if(((nv_card->arch & (NV40 | NV49)) && (nv_card->PMC[0x10f0/4] & 0x80000000)) && !(nv_card->caps & I2C_FANSPEED_MONITORING))
	{
		nv_card->caps |= GPU_FANSPEED_MONITORING;
		nv_card->get_fanspeed = nv40_get_fanspeed;
		nv_card->set_fanspeed = nv40_set_fanspeed;
	}
	else if(((nv_card->arch & (NV41 | NV43 | NV44 | NV47 | NV4B)) && (nv_card->PMC[0x15f4/4] & 0x80000000)) && !(nv_card->caps & I2C_FANSPEED_MONITORING))
	{
		nv_card->caps |= GPU_FANSPEED_MONITORING;
		nv_card->get_fanspeed = nv43_get_fanspeed;
		nv_card->set_fanspeed = nv43_set_fanspeed;
	}
	
	/* Mobile GPU check; we don't want to overclock those unless the user wants it */
	if(nv_card->gpu == MOBILE)
	{
		nv_card->caps = ~(~nv_card->caps | GPU_OVERCLOCKING | MEM_OVERCLOCKING);
	}
	else
		nv_card->caps |= (GPU_OVERCLOCKING | MEM_OVERCLOCKING);
		
	/* Set the speed range */
	if(nv_card->bios)
	{
		/* Most Geforce6 bioses just have one active entry but some Geforce6 6800(Ultra) bioses have 2 entries
		/  in that case the first one contains the highest clocks (3d?). Further there are 6600GT cards with
		/  also two entries for which the second entry contains the 3d clock.
		*/
		if((nv_card->bios->perf_entries == 1) || (nv_card->bios->perf_lst[0].nvclk > nv_card->bios->perf_lst[1].nvclk))
		{
			nv_card->memclk_3d = (short)nv_card->bios->perf_lst[0].memclk;
			nv_card->nvclk_3d = (short)nv_card->bios->perf_lst[0].nvclk;
		}
		else
		{
			/* 6600GT cards have 2d/3d clocks again; the second entries are the 3d ones.
			/  We use the 2d entries for the minimum clocks and the 3d ones for the maximum ones.
			*/
			nv_card->memclk_3d = (short)nv_card->bios->perf_lst[1].memclk;
			nv_card->nvclk_3d = (short)nv_card->bios->perf_lst[1].nvclk;
		}
		nv_card->memclk_min = (short)(nv_card->bios->perf_lst[0].memclk * .75);
		nv_card->memclk_max = nv_card->memclk_3d * 1.25;
		nv_card->nvclk_min = (short)(nv_card->bios->perf_lst[0].nvclk * .75);
		nv_card->nvclk_max = nv_card->nvclk_3d * 1.25;

		/* FIXME: Divide the memory clocks by two on Geforc7600/7900 cards because we program the 'real' clocks for those instead of the effective DDR ones which are twice as high */
		if(nv_card->arch & (NV46 | NV49 | NV4B))
		{
			nv_card->memclk_min /= 2;
			nv_card->memclk_max /= 2;
		}
	}
	else
	{
		float memclk = GetClock_nv40(nv_card->base_freq, nv_card->mpll, nv_card->mpll2);
		float nvclk = GetClock_nv40(nv_card->base_freq, nv_card->nvpll, nv_card->nvpll2);

		/* Not great but better than nothing .. */
		nv_card->memclk_min = (short)(memclk * .75);
		nv_card->memclk_max = (short)(memclk * 1.5);
		nv_card->nvclk_min = (short)(nvclk * .75);
		nv_card->nvclk_max = (short)(nvclk * 1.5);
	}	
}
