/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2005 Roderick Colenbrander
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

#include <stdio.h>
#include <stdlib.h>
#include "backend.h"

static int CalcSpeed(int base_freq, int m, int n, int p)
{
	return (int)((float)n/(float)m * base_freq) >> p;
}

/* Generic clock algorithm for all cards upto the Geforce4 */
float GetClock(int base_freq, unsigned int pll)
{
	int m, n, p;

	m = pll & 0xff;
	n = (pll >> 8) & 0xff;
	p = (pll >> 16) & 0x0f;
	
	if(nv_card->debug)
		printf("m=%d n=%d p=%d\n", m, n, p);
	
	return (float)CalcSpeed(base_freq, m, n, p)/1000;
}

/* Calculate the requested speed. */
static void ClockSelect(int base_freq, int clockIn, int *PLL)
{
	int m, n, p, bestm, bestn, bestp;
	int diff, diffOld, mlow, mhigh, nlow, nhigh, plow, phigh;
	diffOld = clockIn;

	if(base_freq == 14318)
	{
		mlow = 7;
		mhigh = 14;
		nlow = 14;
		nhigh = 255;
	}
	else
	{
		mlow = 6;
		mhigh = 13;
		nlow = 14;
		nhigh = 255;
	}

	if(clockIn > 250000)
	{
		mlow = 1;
		mhigh = 6;
		nlow = 14;
		nhigh = 93;
	}
	if(clockIn > 340000)
	{
		/* When DDR memory is used we should perhaps force mhigh to 1, since
		* on some cards the framebuffer needs to be reinitialized and image corruption
		* can occur.
 		*/
		mlow = 1;
		mhigh = 2;
		nlow = 14;
		nhigh = 93;
	}

	/* postdivider locking to improve stability.
	* in the near future we will provide some tuning options for the
	* overclocking algorithm which will extend this.
	*/
	plow = (*PLL >> 16) & 0x0f;
	phigh = (*PLL >> 16) & 0x0f;

	/*
	Calculate the m and n values. There are a lot of values which give the same speed;
	We choose the speed for which the difference with the request speed is as small as possible.
	*/
	for(p = plow; p <= phigh; p++)
	{
		for(m = mlow; m <= mhigh; m++)
		{
			for(n = nlow; n <= nhigh; n++)
			{
				diff = abs((int)(clockIn - CalcSpeed(base_freq, m, n, p)));

				/* When the new difference is smaller than the old one, use this one */
				if(diff < diffOld)
				{
					diffOld = diff;
					bestm = m;
					bestn = n;
					bestp = p;

#if 0
			/* When the difference is 0 or less than .5% accept the speed */		    
			if(((diff == 0) || ((float)diff/(float)clockIn <= 0.005)))
			{
				*PLL = ((int)bestp << 16) + ((int)bestn << 8) + bestm;
				return;
			}
#endif
				}
			}
		}
	}


	*PLL = ((int)bestp << 16) + ((int)bestn << 8) + bestm;
	return;
}

static void set_gpu_speed(unsigned int clk)
{
	int PLL;

	/* MHz -> KHz */
	clk *= 1000;

	PLL = nv_card->PRAMDAC[0x500/4];

	/* HERE the new clocks are selected (in KHz). */
	ClockSelect(nv_card->base_freq, clk, &PLL);

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Overclock */
	nv_card->PRAMDAC[0x500/4] = PLL;
}

static void set_memory_speed(unsigned int clk)
{
	int PLL = nv_card->PRAMDAC[0x504/4];

	/* MHz -> KHz */
	clk *= 1000;
	/* This is a workaround meant for some Geforce2 MX/Geforce4 MX cards
	*  using SDR memory. Gf2MX/Gf4MX cards use 4x16 SDR memory report
	*  twice as high clockspeeds. I call that "fake ddr".
	*  By detecting the memory type, pci id and clockspeed we check
	*  if this occurs. It is a workaround.
	*/
	if(nv_card->mem_type == SDR && ( nv_card->device_id == 0x110 || nv_card->device_id == 0x111 
	|| nv_card->device_id == 0x172 || nv_card->device_id == 0x17a))
    {
		if(GetClock(nv_card->base_freq, PLL) > 280) clk *= 2;
	}

	/* Here the clocks are selected in kHz */
	ClockSelect(nv_card->base_freq, clk, &PLL);

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Overclock */
	nv_card->PRAMDAC[0x504/4] = PLL;
}

static float get_gpu_speed()
{
	int pll = nv_card->PRAMDAC[0x500/4];

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	if(nv_card->debug == 1)
	{
		printf("NVPLL_COEFF=%08x\n", pll);
	}
	return (float)GetClock(nv_card->base_freq, pll);
}

static float get_memory_speed()
{
	int factor = 1;
	int pll = nv_card->PRAMDAC[0x504/4];

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;
    
	/* This is a workaround meant for some Geforce2 MX/Geforce4 MX cards
	*  using SDR memory. Gf2MX/Gf4MX cards use 4x16 SDR memory report
	*  twice as high clockspeeds. I call that "fake ddr".
	*  By detecting the memory type, pci id and clockspeed we check
	*  if this occurs. It is a workaround. We divide the memclk later by 2.
	*/
	if(nv_card->mem_type == SDR && ( nv_card->device_id == 0x110 || nv_card->device_id == 0x111 ||
	nv_card->device_id == 0x172 || nv_card->device_id == 0x17a || nv_card->device_id == 0x182 \
	|| nv_card->device_id == 0x183))
	{
		if(GetClock(nv_card->base_freq, pll) > 280)
		{
			factor = 2;
		}
	}

	if(nv_card->debug == 1)
	{
		printf("MPLL_COEFF=%08x\n", nv_card->PRAMDAC[0x504/4]);
	}

	return ((float)GetClock(nv_card->base_freq, pll)) / factor;
}

static float nforce_get_memory_speed()
{
	unsigned short p = (pciReadLong(0x3, 0x6c) >> 8) & 0xf;
	if(!p) p = 4;
		return 400.0 / (float)p;
}

static void reset_gpu_speed()
{
	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Set the gpu speed */    
	nv_card->PRAMDAC[0x500/4] = nv_card->nvpll;
}

static void reset_memory_speed()
{
	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Don't overclock the memory of integrated GPUs */
	if(nv_card->gpu == NFORCE)
		return;
	
	/* Set the memory speed */    
	nv_card->PRAMDAC[0x504/4] = nv_card->mpll;
}

static void nv_set_state(int state)
{
}

void nv_init(void)
{
	float memclk, nvclk;

	/* Get the base frequency */
	nv_card->base_freq = (nv_card->PEXTDEV[0x0000/4] & 0x40) ? 14318 : 13500;
	if(nv_card->arch & (NV17 | NV25))
	{
		if (nv_card->PEXTDEV[0x0000/4] & (1<<22))
			nv_card->base_freq = 27000;
	}

	nv_card->set_state = nv_set_state;	
	nv_card->get_gpu_speed = get_gpu_speed;
	nv_card->set_gpu_speed = set_gpu_speed;
	nv_card->get_memory_speed = get_memory_speed;
	nv_card->set_memory_speed = set_memory_speed;
	nv_card->reset_gpu_speed = reset_gpu_speed;
	nv_card->reset_memory_speed = reset_memory_speed;

	/* Register I2C busses for hardware monitoring purposes */
	if(nv_card->busses[0] == NULL)
	{
		nv_card->num_busses = 2;
		nv_card->busses[0] = NV_I2CCreateBusPtr("BUS0", 0x3e); /* available on riva128 and higher */
		nv_card->busses[1] = NV_I2CCreateBusPtr("BUS1", 0x36); /* available on rivatnt hardware and  higher */

		/* There's an extra bus available on geforce4mx/ti, geforcefx and geforce6 cards.
		/  The check below looks for geforce4mx/geforcefx/geforce6 architecture.
		*/
		if(nv_card->arch & (NV17 | NV25 | NV3X | NV4X))
		{
			nv_card->num_busses = 3;
			nv_card->busses[2] = NV_I2CCreateBusPtr("BUS2", 0x50); 
		}
	}

	/* Mobile GPU check; we don't want to overclock those unless the user wants it */
	if(nv_card->gpu == MOBILE)
	{
		nv_card->caps = ~(~nv_card->caps | GPU_OVERCLOCKING | MEM_OVERCLOCKING);
	}
	else if(nv_card->gpu == NFORCE)
	{
		/* Only support gpu overclocking because the memory is normal system memory of which we can't adjust the clocks */
		nv_card->caps |= GPU_OVERCLOCKING;
		nv_card->get_memory_speed = nforce_get_memory_speed;
		nv_card->set_memory_speed = NULL;
		nv_card->reset_memory_speed = NULL;
	}
	else
		nv_card->caps |= (GPU_OVERCLOCKING | MEM_OVERCLOCKING);

	/* Set the speed range */
	memclk = GetClock(nv_card->base_freq, nv_card->mpll);
	nvclk = GetClock(nv_card->base_freq, nv_card->nvpll);
	nv_card->memclk_min = (short)(memclk * .75);
	nv_card->memclk_max = (short)(memclk * 1.5);
	nv_card->nvclk_min = (short)(nvclk * .75);
	nv_card->nvclk_max = (short)(nvclk * 1.5);

	/* Find out what memory is being used */
	nv_card->mem_type = (nv_card->PFB[0x200/4] & 0x01) ? DDR : SDR;
	
	/* Hack.
	* Nvidia was so nice to ship support both DDR and SDR memory on some gf2mx and gf4mx cards :(
	* Because of this the speed ranges of the memory speed can be different.
	* Check if the card is a gf2mx/gf4mx using SDR and if the speed is "too" high.
	* Then adjust the speed range.
	*/
	if((nv_card->device_id == 0x110 || nv_card->device_id == 0x111 \
		|| nv_card->device_id == 0x172 || nv_card->device_id == 0x182 \
		|| nv_card->device_id == 0x183) && ((nv_card->PFB[0x200/4] & 0x1) == SDR) && memclk > 280)
	{
		nv_card->memclk_min /= 2;
		nv_card->memclk_max /= 2;
	}
}
