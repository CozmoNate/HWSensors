//
//  gm100.cpp
//  HWSensors
//
//  Created by Kozlek on 05.04.14.
//
//


/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */

#include "gm100.h"

#include "nouveau.h"
#include "nv50.h"
#include "nva3.h"
#include "nv84.h"
#include "nvd0.h"
#include "nve0.h"
#include "nouveau_therm.h"

bool gm100_identify(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0x117:
            device->cname = "GM107";
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] =  nve0_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] =  nvd0_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_FUSE   ] = &gm107_fuse_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nve0_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_THERM  ] = &gm107_therm_oclass;
//            device->oclass[NVDEV_SUBDEV_MXM    ] = &nv50_mxm_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] =  gm107_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] =  gk20a_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_BUS    ] =  nvc0_bus_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &gk20a_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] =  gm107_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_LTC    ] =  gm107_ltc_oclass;
//            device->oclass[NVDEV_SUBDEV_IBUS   ] = &nve0_ibus_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] =  nv50_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nvc0_vmmgr_oclass;
//            device->oclass[NVDEV_SUBDEV_BAR    ] = &nvc0_bar_oclass;
//            device->oclass[NVDEV_SUBDEV_PWR    ] =  nv108_pwr_oclass;
//
//#if 0
//            device->oclass[NVDEV_SUBDEV_VOLT   ] = &nv40_volt_oclass;
//#endif
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] =  nvd0_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] =  nv108_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] =  nvc0_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] =  gm107_graph_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] =  gm107_disp_oclass;
//            device->oclass[NVDEV_ENGINE_COPY0  ] = &nve0_copy0_oclass;
//#if 0
//            device->oclass[NVDEV_ENGINE_COPY1  ] = &nve0_copy1_oclass;
//#endif
//            device->oclass[NVDEV_ENGINE_COPY2  ] = &nve0_copy2_oclass;
//#if 0
//            device->oclass[NVDEV_ENGINE_BSP    ] = &nve0_bsp_oclass;
//            device->oclass[NVDEV_ENGINE_VP     ] = &nve0_vp_oclass;
//            device->oclass[NVDEV_ENGINE_PPP    ] = &nvc0_ppp_oclass;
//#endif
            break;

        default:
            nv_fatal(device, "unknown Maxwell chipset 0x%x\n", device->chipset);
            return false;
	}

	return true;
}

static int gm107_fan_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty)
{
    *divs = nv_rd32(device, 0x10eb20) & 0x1fff;
    *duty = nv_rd32(device, 0x10eb24) & 0x1fff;
    return 0;
}

void gm100_init(struct nouveau_device *device)
{
    nvd0_therm_init(device);

    device->gpio_find = nouveau_gpio_find;
    device->gpio_get = nouveau_gpio_get;
    device->gpio_sense = nvd0_gpio_sense;
    device->temp_get = nv84_temp_get;
    device->clocks_get = nve0_clock_read;
    //    device->voltage_get = nouveau_voltage_get;
    device->pwm_get = gm107_fan_pwm_get;
    device->fan_pwm_get = nouveau_therm_fan_pwm_get;
    device->fan_rpm_get = nva3_therm_fan_sense;
}
