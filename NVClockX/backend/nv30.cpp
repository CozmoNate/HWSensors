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

#include <stdio.h>
#include "backend.h"

static float nv30_get_fanspeed()
{
	/* Bit 30-16 of register 0x10f0 are used to store the fanspeed and bit 14-0 contain the pwm divider */
	int pwm_divider = nv_card->PMC[0x10f0/4] & 0x7fff;
	return ((nv_card->PMC[0x10f0/4] >> 16) & 0x7fff) * 100.0/pwm_divider;
}

static void nv30_set_fanspeed(float speed)
{
	int value;
	int pwm_divider = nv_card->PMC[0x10f0/4] & 0x7fff;

	/* Don't set the fan lower than 10% for safety reasons */
	if(speed < 10 || speed > 100)
		return;

	value = 0x80000000 + (((int)(speed * pwm_divider/100) & 0x7fff) << 16) + (nv_card->PMC[0x10f0/4] & 0xffff);
	nv_card->PMC[0x10f0/4] = value;
}

static int CalcSpeed(int base_freq, int m1, int m2, int n1, int n2, int p)
{
	return (int)((float)(n1*n2)/(m1*m2) * base_freq) >> p;
}

float GetClock_nv30(int base_freq, unsigned int pll, unsigned int pll2)
{
	int m1, m2, n1, n2, p;

	/* Geforce FX5600 cards and the FX5700?? (0x340) use an additional PLL register */
	if(nv_card->arch & NV31)
	{
		m1 = pll & 0xff;
		n1 = (pll >> 8) & 0xff;
		p = (pll >> 16) & 0x0f;
	
		/* For some reasons a speed consists of two plls */
		if(pll2 & 0x80000000)
		{
			m2 = pll2 & 0xff;
			n2 = (pll2 >> 8) & 0xff;
		}
		else
		{
			m2 = 1;
			n2 = 1;
		}
	}
	else
	{
		/* All GeforceFX cards except the FX5200/FX5600/FX5700 use this algorithm */
		/* If bit 7 is set use two VCOs else use the old algorithm; do cards fall back to a single PLL? */
		if(pll & 0x80)
		{
			m1 = NV30_PLL_M1(pll);
			m2 = NV30_PLL_M2(pll);
			n1 = NV30_PLL_N1(pll);
			n2 = NV30_PLL_N2(pll); /* Perhaps the 0x1f for n2 is wrong .. ? */
			p = NV30_PLL_P(pll);
		}
		else
		{
			m1 = NV30_PLL_M1(pll);
			m2 = 1;
			n1 = NV30_PLL_N1(pll);
			n2 = 1;
			p = NV30_PLL_P(pll);
		}
	}

	if(nv_card->debug)
		printf("m1=%d m2=%d n1=%d n2=%d p=%d\n", m1, m2, n1, n2, p);

	return (float)CalcSpeed(base_freq, m1, m2, n1, n2, p)/1000;
}

static void ClockSelect_nv30(int clockIn, int p_current, int *bestM1, int *bestM2, int *bestN1, int *bestN2, int *bestP)
{
	unsigned diff, diffOld;
	unsigned VClk, Freq;
	unsigned m, m2, n, n2, p = 0;
	int base_freq = 27000;

	diffOld = 0xFFFFFFFF;

	if(clockIn < 125)
		p = 3;
	else if(clockIn < 250)
		p = 2;
	else if(clockIn < 500)
		p = 1;
	else
		p = 0;
	
	VClk = (unsigned)clockIn;

	Freq = VClk;
	if ((Freq >= 75000) && (Freq <= 1100000))
	{
		for(m = 1; m <= 4; m++)
		{
			for (m2 = 1; m2 <= 4; m2++)
			{
				for(n = 1; n <= 31; n++)
				{
					n2 = (int)((float)((VClk << p) * m * m2) / (float)(base_freq * n)+.5);

					if((n2 < 24) && (n >= n2) && (m >= m2))
					{
						Freq = ((base_freq * n * n2) / (m * m2)) >> p;
						if (Freq > VClk)
							diff = Freq - VClk;
						else
							diff = VClk - Freq;

						/* When the difference is 0 or less than .5% accept the speed */
						if( (float)diff/(float)clockIn <= 0.005)
						{
							*bestM1 = m;
							*bestM2 = m2;
							*bestN1 = n;
							*bestN2 = n2;
							*bestP = p;
							return;
						}
						if(diff < diffOld)
						{
							*bestM1 = m;
							*bestM2 = m2;
							*bestN1 = n;
							*bestN2 = n2;
							*bestP = p;
						}
					}
				}
			}
		}
	}
}

static float nv30_get_gpu_speed()
{
	int pll = nv_card->PRAMDAC[0x500/4];
	int pll2 = nv_card->PRAMDAC[0x570/4];
	if(nv_card->debug == 1)
	{
		printf("NVPLL_COEFF=%08x\n", pll);
		printf("NVPLL_COEFF2=%08x\n", pll2);
	}

	return (float)GetClock_nv30(nv_card->base_freq, pll, pll2);
}

static void nv30_set_gpu_speed(unsigned int nvclk)
{
	unsigned int PLL=0;
	int m1, m2, n1, n2, p;
	nvclk *= 1000;

	p = NV30_PLL_P(nv_card->PRAMDAC[0x500/4]);
	ClockSelect_nv30(nvclk, p, &m1, &m2, &n1, &n2, &p);
	PLL = m1 + (m2<<4) + (n1<<8) + ((n2 & 0x7) << 19) + ((n2 & 0x18)<<21) + (p<<16) + (1<<7);

	if(nv_card->debug)
	{
		printf("NVPLL_COEFF: %08x\n", PLL);
	}

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;
	
	if(PLL)
		nv_card->PRAMDAC[0x500/4] = PLL;
}

static void nv31_set_gpu_speed(unsigned int nvclk)
{
	unsigned int PLL, PLL2;
	int m1, m2, n1, n2, p;
	nvclk *= 1000;

	p = NV30_PLL_P(nv_card->PRAMDAC[0x500/4]);
	ClockSelect_nv30(nvclk, p, &m1, &m2, &n1, &n2, &p);
	PLL  = (p << 16) | (n1 << 8) | m1;
	PLL2 = (1<<31) | (n2 << 8) | m2;

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* When no speed is found, don't change the PLL */
	/* The algorithm doesn't allow too low speeds */
	if(PLL)
	{
		if(nv_card->debug)
		{
			printf("NVPLL_COEFF: %08x\n", PLL);
			printf("NVPLL2_COEFF: %08x\n", PLL2);
		}
		nv_card->PRAMDAC[0x500/4] = PLL;
		nv_card->PRAMDAC[0x570/4] = PLL2;
	}
}

static float nv30_get_memory_speed()
{
	int pll = nv_card->PRAMDAC[0x504/4];
	int pll2 = nv_card->PRAMDAC[0x574/4];
	if(nv_card->debug == 1)
	{
		printf("MPLL_COEFF=%08x\n", pll);
		printf("MPLL_COEFF2=%08x\n", pll2);
	}

	return (float)GetClock_nv30(nv_card->base_freq, pll, pll2);
}

static void nv30_set_memory_speed(unsigned int memclk)
{
	unsigned int PLL=0;
	int m1, m2, n1, n2, p;
	memclk *= 1000;

	p = NV30_PLL_P(nv_card->PRAMDAC[0x500/4]);
	ClockSelect_nv30(memclk, p, &m1, &m2, &n1, &n2, &p);
	PLL = m1 + (m2<<4) + (n1<<8) + ((n2 & 0x7) << 19) + ((n2 & 0x18)<<21) + (p<<16) + (1<<7);

	if(nv_card->debug)
		printf("MPLL_COEFF: %08x\n", PLL);

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;
	if(PLL)
		nv_card->PRAMDAC[0x504/4] = PLL;
}

static void nv31_set_memory_speed(unsigned int memclk)
{
	unsigned int PLL, PLL2;
	int m1, m2, n1, n2, p;
	memclk *= 1000;

	p = NV30_PLL_P(nv_card->PRAMDAC[0x500/4]);
	ClockSelect_nv30(memclk, p, &m1, &m2, &n1, &n2, &p);
	PLL  = (p << 16) | (n1 << 8) | m1;
	PLL2 = (1<<31) | (n2 << 8) | m2;

	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* When no speed is found, don't change the PLL */
	/* The algorithm doesn't allow too low speeds */
	if(PLL)
	{
		if(nv_card->debug)
		{
			printf("MPLL_COEFF: %08x\n", PLL);
			printf("MPLL2_COEFF: %08x\n", PLL2);
		}
		nv_card->PRAMDAC[0x504/4] = PLL;
		nv_card->PRAMDAC[0x574/4] = PLL2;
	}
}

static void nv30_reset_gpu_speed()
{
	/* FIXME: we need to use our bios info */
	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Set the gpu speed */    
	nv_card->PRAMDAC[0x500/4] = nv_card->nvpll;
}

static void nv31_reset_gpu_speed()
{
	/* FIXME: we need to use our bios info */
	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Set the gpu speed */
	nv_card->PRAMDAC[0x500/4] = nv_card->nvpll;
	nv_card->PRAMDAC[0x570/4] = nv_card->nvpll2;
}

static void nv30_reset_memory_speed()
{
	/* FIXME: we need to use our bios info */
	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Set the memory speed */    
	nv_card->PRAMDAC[0x504/4] = nv_card->mpll;
}

static void nv31_reset_memory_speed()
{
	/* FIXME: we need to use our bios info */
	/* Unlock the programmable NVPLL/MPLL */
	nv_card->PRAMDAC[0x50c/4] |= 0x500;

	/* Set the memory speed */
	nv_card->PRAMDAC[0x504/4] = nv_card->mpll;
	nv_card->PRAMDAC[0x574/4] = nv_card->mpll2;
}

static void nv30_set_state(int state)
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
		if(nv_card->arch & NV31)
		{
			nv_card->get_gpu_speed = nv30_get_gpu_speed;
			nv_card->get_memory_speed = nv30_get_memory_speed;
			nv_card->set_memory_speed = nv31_set_memory_speed;
			nv_card->set_gpu_speed = nv31_set_gpu_speed;
			nv_card->reset_gpu_speed = nv31_reset_gpu_speed;
			nv_card->reset_memory_speed = nv31_reset_memory_speed;
		}
		else /* FX5800/5900 */
		{
			nv_card->get_gpu_speed = nv30_get_gpu_speed;
			nv_card->get_memory_speed = nv30_get_memory_speed;
			nv_card->set_memory_speed = nv30_set_memory_speed;
			nv_card->set_gpu_speed = nv30_set_gpu_speed;
			nv_card->reset_gpu_speed = nv30_reset_gpu_speed;
			nv_card->reset_memory_speed = nv30_reset_memory_speed;
		}
	}
}

void nv30_init(void)
{
	nv_card->base_freq = 27000;

	nv_card->set_state = nv30_set_state;
	nv_card->set_state(STATE_LOWLEVEL); /* Set the clock function pointers */

	/* Register I2C busses for hardware monitoring purposes */
	if(nv_card->busses[0] == NULL)
	{
		nv_card->num_busses = 3;
		nv_card->busses[0] = NV_I2CCreateBusPtr("BUS0", 0x3e); /* available on riva128 and higher */
		nv_card->busses[1] = NV_I2CCreateBusPtr("BUS1", 0x36); /* available on rivatnt hardware and  higher */
		nv_card->busses[2] = NV_I2CCreateBusPtr("BUS2", 0x50);  /* available on geforce4mx/4ti/fx/6/7 */

		i2c_sensor_init();
	}

	/* HW monitoring; bit 31 is an indication if fanspeed monitoring is available
	/  Note this bit isn't very reliable as it is set on cards with advanced sensors too.
	/
	/  Only support this on NV30/NV35/NV38 hardware for now as it works differently on other NV3x boards
	*/
	if((nv_card->PMC[0x10f0/4] & 0x80000000) && (nv_card->arch & (NV30 | NV35)) && !(nv_card->caps & I2C_FANSPEED_MONITORING))
	{
		nv_card->caps |= GPU_FANSPEED_MONITORING;
		nv_card->get_fanspeed = nv30_get_fanspeed;
		nv_card->set_fanspeed = nv30_set_fanspeed;
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
		/* GeforceFX models have different clocks in 2d and 3d; above hack doesn't work for those */
		nv_card->memclk_min = (short)(nv_card->bios->perf_lst[0].memclk * .75);
		nv_card->memclk_max = (short)(nv_card->bios->perf_lst[2].memclk * 1.25);
		nv_card->nvclk_min = (short)(nv_card->bios->perf_lst[0].nvclk * .75);
		nv_card->nvclk_max = (short)(nv_card->bios->perf_lst[2].nvclk * 1.25);
	}
	else
	{
		float memclk = GetClock_nv30(nv_card->base_freq, nv_card->mpll, nv_card->mpll2);
		float nvclk = GetClock_nv30(nv_card->base_freq, nv_card->nvpll, nv_card->nvpll2);

		/* Not great but better than nothing .. */
		nv_card->memclk_min = (short)(memclk * .75);
		nv_card->memclk_max = (short)(memclk * 1.5);
		nv_card->nvclk_min = (short)(nvclk * .75);
		nv_card->nvclk_max = (short)(nvclk * 1.5);
	}
}
