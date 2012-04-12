/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2007 Roderick Colenbrander
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
#include <string.h>
#include "backend.h"

static void nv50_i2c_get_bits(I2CBusPtr bus, int *clock, int *data)
{
	const int offset = bus->DriverPrivate.val;
	unsigned char val;

	val = nv_card->PMC[(0x0000E138 + offset)/4];
	*clock = !!(val & 1);
	*data = !!(val & 2);
}

static void nv50_i2c_put_bits(I2CBusPtr bus, int clock, int data)
{
	const int offset = bus->DriverPrivate.val;
	nv_card->PMC[(0x0000E138 + offset)/4] = 4 | clock | data << 1;
}

static I2CBusPtr nv50_i2c_create_bus_ptr(char *name, int bus)
{
	I2CBusPtr I2CPtr;

	I2CPtr = xf86CreateI2CBusRec();
	if(!I2CPtr) return NULL;

	I2CPtr->BusName    = name;
	I2CPtr->scrnIndex  = nv_card->number; /* We need to use unique indices or else it can lead to a segfault in multicard situations */
	I2CPtr->I2CAddress = I2CAddress;
	I2CPtr->I2CPutBits = nv50_i2c_put_bits;
	I2CPtr->I2CGetBits = nv50_i2c_get_bits;
	I2CPtr->AcknTimeout = 40;
	I2CPtr->DriverPrivate.val = bus;

	if (!xf86I2CBusInit(I2CPtr))
	{
		return 0;
	}
	return I2CPtr;
}

static int nv50_get_default_mask(char *smask, char *rmask)
{
	int mask;
	switch(nv_card->arch)
	{
	case NV50:
		mask = 0x3f00ff;
		break;
	case G84:
		mask = 0x030003;
		break;
	case G86:
		mask = 0x010001;
		break;
	case G92:
		mask = 0x0f00ff;
		break;
	case G94:
		mask = 0x0f000f;
		break;
	case GT200:
		mask = 0xff03ff;
		break;
	}

	if(smask)
		*smask = mask & 0x3ff;
	if(rmask)
		*rmask = (mask >> 16) & 0xff;

	return mask;
}

/* Receive the number of enabled stream processors and also
   /  store a mask with active pipelines.
*/
static int nv50_get_stream_units(char *mask, char *default_mask)
{
	int i, stream_units=0;
	int32_t stream_units_cfg = nv_card->PMC[0x1540/4] & 0x3ff;
	/* The number of shaders is again stored in 0x1540
	   bit9-0: number of unified pixel shaders in blocks of 16
	   bit23-16: number of ROP units in blocks of 4
	   bit31-24: what's in here?
	*/

	for(i=0; i<10; i++)
		if((stream_units_cfg >> i) & 0x1)
			stream_units++;

	nv50_get_default_mask(default_mask, 0);

	*mask = stream_units_cfg;

	if(nv_card->arch == GT200)
		return (stream_units * 24); /* GT200 stores stream units in blocks of 24 */

	return (stream_units << 4); /* stream units are stored in blocks of 16 */
}

/* Receive the number of enabled ROP units and also
   /  store a mask with active units.
*/
static int nv50_get_rop_units(char *mask, char *default_mask)
{
	int i, rop_units=0;
	unsigned char rop_units_cfg = (nv_card->PMC[0x1540/4] >> 16) & 0xff;

	for(i=0; i<8; i++)
		if((rop_units_cfg >> i) & 0x1)
			rop_units++;

	nv50_get_default_mask(0, default_mask);

	*mask = rop_units_cfg;
	return (rop_units << 2); /* rop units are stored in blocks of 4 */
}

static float g84_get_fanspeed()
{
	int pwm_divider = nv_card->PMC[0xe11c/4] & 0x7fff;

	/* On most Geforce8/9 cards I have seen the fanspeed register is 'inverted', so
	   /  a low value corresponds with fullspeed (to be exact the register defines the low
	   /  period of a pwm pulse. Though some boards aren't inverted like a 8500GT (G86). I'm
	   /  not sure what we should do about this. If it is possible to whitelist some generations
	   /  or so we should perhaps do that or perhaps there is some setting in the bios? So right
	   /  now 100% would show 0% on a 8500GT.
	   /
	   /  Further some boards use 0xe114 / 0xe118 instead of 0xe11c / 0xe1220. At least the 9800GTX
	   /  seems to do that. When I have a more clear picture of the situation those should receive support too.
	*/
	float fanspeed = (float)(pwm_divider - (nv_card->PMC[0xe120/4] & 0x7fff)) * 100.0/(float)pwm_divider;
	return fanspeed;
}

static void g84_set_fanspeed(float speed)
{
	int value;
	int pwm_divider = nv_card->PMC[0xe11c/4] & 0x7fff;

	/* For safety reasons we should never disable the fan by not putting it below 10%; further negative values don't exist ;)  */
	if(speed < 10 || speed > 100)
		return;

	/* Bit31 must be set else the hardware doesn't seem to do anything with the changes
	   /  Bit30-16 contain some magical bits on 9500GT and other cards which we should preserve.
	   /  On a 9500gt the contents of 0xe120 could be e.g. 0x0300010e with a pwm_divider of 0x21d.
	   /
	   /  Note Oxe300 is also related to the fanspeed. By default it seems to contain 0x100 on
	   /  9600GT and other cards. Setting this value to 0x300 seems to set the fanspeed to a fixed
	   /  value. Apparently 0xe300 acts like a multiplexer?
	*/
	value = 0x80000000 | (nv_card->PMC[0xe120/4] & 0x7fff0000) | (((int)(100 - speed) * pwm_divider/100) & 0x7fff);
	nv_card->PMC[0xe120/4] = value;
}

/* Reading of the internal gpu sensor, it not entirely correct yet */
static int nv50_get_gpu_temp(void *sensor)
{
	int temp;
	int correction=0;
	float offset;
	float slope;

	/* For now use a hardcoded offset and gain. This isn't correct but I don't know how the new temperture table works yet; this at least gives something */
	offset = -227.0;
	slope = 430.0/10000.0;

	if (nv_card->arch & (G92 | GT200)) {
		correction = nv_card->bios->sensor_cfg.temp_correction<<2;
	}

	if(nv_card->debug)
	{
		printf("NV_20008 (0x20008): %08x\n", nv_card->PMC[0x20008/4]);
		printf("slope=%f, offset=%f, correction=%d\n", slope, offset, correction);
	}

	temp = nv_card->PMC[0x20008/4] & 0x1fff;
	return (int)(temp * slope + offset) + correction;
}

static int g84_get_gpu_temp(void *sensor)
{
	if(nv_card->debug)
	{
		/* A calibrated value of the temperature is stored in 0x20400, raw in 0x20008 it would require bios info to calibrate it */
		printf("NV_20008 (0x20008): %08x\n", nv_card->PMC[0x20008/4]);
		printf("NV_20400 (0x20400): %08x\n", nv_card->PMC[0x20400/4]);
	}

	/* A calibrated value of the temperature is stored in 0x20400 */
	return nv_card->PMC[0x20400/4];
}

static int g92_get_gpu_temp(void *sensor)
{
	int temp;
	float offset;
	float divider;

	/* According to a formula on the rivatuner forum Asus uses the following formula: (-13115 + raw_temp)/18.7 + 1. */
	offset = -13115 + 18.7;
	divider = 18.7;

	if(nv_card->debug)
	{
		printf("NV_20008 (0x20008): %08x\n", nv_card->PMC[0x20008/4]);
		printf("divider=%f, offset=%f\n", divider, offset);
	}

	temp = nv_card->PMC[0x20008/4] & 0x1fff;
	return (int)(temp + offset)/divider;
}


/* Get current backpanel brightness level on laptops */
static int nv50_mobile_get_smartdimmer()
{
	int val = nv_card->PDISPLAY[NV_PDISPLAY_SOR0_REGS_BRIGHTNESS/4];
	/* Convert level to a value between 1 and 100 */
	/* For now assume the maximum value is 1024 */
	return ((val / 1024.0 * 100) + 0.5);
}

/* Adjust backpanel brightness on laptops */
static void nv50_mobile_set_smartdimmer(int level)
{
	int val;
	if(level < 15 || level > 100)
		return;
	val = (level * 1024 / 100) | NV_PDIPSLAY_SOR0_REGS_BRIGHTNESS_CONTROL_ENABLED;
	nv_card->PDISPLAY[NV_PDISPLAY_SOR0_REGS_BRIGHTNESS/4] = val;
}

static int CalcSpeed_nv50(int base_freq, int m1, int m2, int n1, int n2, int p)
{
	return (int)((float)(n1*n2)/(m1*m2) * base_freq) >> p;
}

float GetClock_nv50(int base_freq, unsigned int pll, unsigned int pll2)
{
	int m1, m2, n1, n2, p;

	p = (pll >> 16) & 0x03;
	m1 = pll2 & 0xFF;
	n1 = (pll2 >> 8) & 0xFF;

	/* This is always 1 for NV5x? */
	m2 = 1;
	n2 = 1;

	if(nv_card->debug)
		printf("m1=%d m2=%d n1=%d n2=%d p=%d\n", m1, m2, n1, n2, p);

	/* The clocks need to be multiplied by 4 for some reason. Is this 4 stored in 0x4000/0x4004? */
	return (float)4*CalcSpeed_nv50(base_freq, m1, m2, n1, n2, p)/1000;
}

static float nv50_get_gpu_speed()
{
	int pll = nv_card->PMC[0x4028/4];
	int pll2 = nv_card->PMC[0x402c/4];
	if(nv_card->debug == 1)
	{
		printf("NVPLL_COEFF=%08x\n", pll);
		printf("NVPLL2_COEFF=%08x\n", pll2);
	}

	return (float)GetClock_nv50(nv_card->base_freq, pll, pll2);
}

static void nv50_set_gpu_speed(unsigned int nvclk)
{
}

static float nv50_get_memory_speed()
{
	/* The memory clock appears to be in 0x4008/0x400c, 0x4010/0x4014 and 0x4018/0x401c but the second and third set aren't always set to the same values as 0x4008/0x400c */ 
	int pll = nv_card->PMC[0x4008/4];
	int pll2 = nv_card->PMC[0x400c/4];
	if(nv_card->debug == 1)
	{
		printf("MPLL_COEFF=%08x\n", pll);
		printf("MPLL2_COEFF=%08x\n", pll2);
	}

	return (float)GetClock_nv50(nv_card->base_freq, pll, pll2);
}

static void nv50_set_memory_speed(unsigned int memclk)
{
}

static float nv50_get_shader_speed()
{
	int pll = nv_card->PMC[0x4020/4];
	int pll2 = nv_card->PMC[0x4024/4];
	if(nv_card->debug == 1)
	{
		printf("SPLL_COEFF=%08x\n", pll);
		printf("SPLL2_COEFF=%08x\n", pll2);
	}

	return (float)GetClock_nv50(nv_card->base_freq, pll, pll2);
}

static void nv50_set_shader_speed(unsigned int clk)
{
}

static void nv50_reset_gpu_speed()
{
}

static void nv50_reset_memory_speed()
{
}

static void nv50_reset_shader_speed()
{
}

static void nv50_set_state(int state)
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
		nv_card->get_gpu_speed = nv50_get_gpu_speed;
		nv_card->get_memory_speed = nv50_get_memory_speed;
		nv_card->set_memory_speed = nv50_set_memory_speed;
		nv_card->set_gpu_speed = nv50_set_gpu_speed;
		nv_card->reset_gpu_speed = nv50_reset_gpu_speed;
		nv_card->reset_memory_speed = nv50_reset_memory_speed;
	}
	nv_card->get_shader_speed = nv50_get_shader_speed;
	nv_card->set_shader_speed = nv50_set_shader_speed;
	nv_card->reset_shader_speed = nv50_reset_shader_speed;
}

void nv50_init(void)
{
	nv_card->base_freq = 27000;

	nv_card->set_state = nv50_set_state;
	nv_card->set_state(STATE_LOWLEVEL); /* Set the clock function pointers */	

	nv_card->get_stream_units = nv50_get_stream_units;
	nv_card->get_rop_units = nv50_get_rop_units;

	/* Initialize the NV50 I2C busses; compared to older hardware they are located at different register addresses */
	if(nv_card->busses[0] == NULL)
	{
		nv_card->num_busses = 4;
		nv_card->busses[0] = nv50_i2c_create_bus_ptr("BUS0", 0x0);
		nv_card->busses[1] = nv50_i2c_create_bus_ptr("BUS1", 0x18);
		nv_card->busses[2] = nv50_i2c_create_bus_ptr("BUS2", 0x30);
		nv_card->busses[3] = nv50_i2c_create_bus_ptr("BUS3", 0x48);

		i2c_sensor_init();
	}

	/* For now unlock laptops of HP, Samsung (Sanyo), Sony and Zepto. Later on we whould find a better way if there is any.
	   /  Most likely there is some info in the bios which can help us figuring out what smartdimmer method is used.
	   /  The code has been reported to work on the following models:
	   / - Apple Macbook5,1 (Aluminium), Geforce 9400M, dev=0x863, subvendor=0x106b
	   / - HP Compaq 8510W, QuadroFX 570M, dev=0x40c, subvendor=0x103c
	   / - HP Compaq 8710P, QuadroFX 320M, dev=0x40b, subvendor=0x103c
	   / - Samsung Q210 / Q310 / R510, Geforce 9200M, dev=0x6e8, subvendor=0x144d
	   / - Sony Vaio models using 8400/8600 GPUs:
	   /   VGN SZ650N, SZ61MN/B, SZ730E, SZ750N, SZ71MN/B, SZ71E, VGN FZ38M, FZ31M, FZ11Z, NR31, AR41E, FZ11S, FZ290, FZ250AE,
	   /   FZ21E, FZ21M, FZ470E, FZ340E, FZ190N, FZ18M, FZ31E, FZ18E, FZ31Z, FZ21Z, FZ31S, AR51SU, AR71S
	   / - Zepto (unknown model), Geforce 9600M, dev=0x649, subvendor=0x1a46, 
	*/
	if((nv_card->gpu == MOBILE) &&
	   ((nv_card->subvendor_id == PCI_VENDOR_ID_APPLE) ||
	    (nv_card->subvendor_id == PCI_VENDOR_ID_HP) ||
	    (nv_card->subvendor_id == PCI_VENDOR_ID_SANYO) ||
	    (nv_card->subvendor_id == PCI_VENDOR_ID_SONY) ||
	    (nv_card->subvendor_id == PCI_VENDOR_ID_ZEPTO)))
	{
		nv_card->caps |= SMARTDIMMER;
		nv_card->get_smartdimmer = nv50_mobile_get_smartdimmer;
		nv_card->set_smartdimmer = nv50_mobile_set_smartdimmer;
	}

	/* Temperature monitoring; all NV50 cards feature an internal temperature sensor
	   /  but only use it when there is no I2C sensor around.
	*/
	if((nv_card->arch & NV50) && !(nv_card->caps & GPU_TEMP_MONITORING))
	{
		nv_card->caps |= GPU_TEMP_MONITORING;
		nv_card->sensor_name = (char*)strdup("GPU Internal Sensor");
		nv_card->get_gpu_temp = (int(*)(I2CDevPtr))nv50_get_gpu_temp;
	}
	else if((nv_card->arch & (G84 | G86 | G94 | G96)) && !(nv_card->caps & GPU_TEMP_MONITORING))
	{
		nv_card->caps |= GPU_TEMP_MONITORING;
		nv_card->sensor_name = (char*)strdup("GPU Internal Sensor");
		nv_card->get_gpu_temp = (int(*)(I2CDevPtr))g84_get_gpu_temp;
	}
	else if((nv_card->arch & (G92 | GT200)) && !(nv_card->caps & GPU_TEMP_MONITORING))
	{
		/* Nearly all G92 boards use a ADT7473 except some Asus models. They don't use the bios data properly, so give it its own function */
		nv_card->caps |= GPU_TEMP_MONITORING;
		if (g92_get_gpu_temp(0)>0) {
			nv_card->sensor_name = (char*)strdup("ASUS GPU Internal Sensor");
			nv_card->get_gpu_temp = (int(*)(I2CDevPtr))g92_get_gpu_temp;
		} else if (g84_get_gpu_temp(0)>0) {
			nv_card->sensor_name = (char*)strdup("G84 GPU Internal Sensor");
			nv_card->get_gpu_temp = (int(*)(I2CDevPtr))g84_get_gpu_temp;
		} else if (nv50_get_gpu_temp(0)>0) {
			nv_card->sensor_name = malloc(64);
			sprintf(nv_card->sensor_name,"NV50 GPU Internal Sensor (correction=%d)",
				nv_card->bios->sensor_cfg.temp_correction<<2);
			nv_card->get_gpu_temp = (int(*)(I2CDevPtr))nv50_get_gpu_temp;
		}

	}


	/* Fanspeed adjustment support for several 8600GT 'G84', 9600GT 'G94' and 9600GSO 'G92' boards
	   /  For now only support fanspeed adjustment using pwm registers 0xe11c/0xe120. Assume they are
	   /  active when 0xe11c (pwm divider) is higher than 1. We need a proper method to distinguish between
	   /  the use of 0xe114/0xe118 and 0xe11c/0xe120 and to detect whether the pwm signal needs to be inverted
	   /  or not. Likely there is info in the bios for this.
	*/
	if((nv_card->arch & (G84 | G86 | G92 | G94 | G96)) && !(nv_card->caps & I2C_FANSPEED_MONITORING) && (nv_card->PMC[0xe11c/4] > 1))
	{
		nv_card->caps |= GPU_FANSPEED_MONITORING;
		nv_card->get_fanspeed = g84_get_fanspeed;
		nv_card->set_fanspeed = g84_set_fanspeed;
	}

	/* Mobile GPU check; we don't want to overclock those unless the user wants it */
	if(nv_card->gpu == MOBILE)
	{
		nv_card->caps = ~(~nv_card->caps | GPU_OVERCLOCKING | MEM_OVERCLOCKING);
	}
	else
		nv_card->caps |= (GPU_OVERCLOCKING | MEM_OVERCLOCKING);

#if 0		
	/* Set the speed range */
	if(nv_card->bios)
	{

	}
	else
#endif
	{
		float memclk = GetClock_nv50(nv_card->base_freq, nv_card->mpll, nv_card->mpll2);
		float nvclk = GetClock_nv50(nv_card->base_freq, nv_card->nvpll, nv_card->nvpll2);

		/* Not great but better than nothing .. */
		nv_card->memclk_min = (short)(memclk * .75);
		nv_card->memclk_max = (short)(memclk * 1.5);
		nv_card->nvclk_min = (short)(nvclk * .75);
		nv_card->nvclk_max = (short)(nvclk * 1.5);
	}	
}
