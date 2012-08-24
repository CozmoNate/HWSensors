//
//  nve0.c
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
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

#include "nouveau.h"
#include "nv50.h"
#include "nve0.h"
#include "nvd0.h"

bool nve0_identify(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0xe4:
            device->cname = "GK104";
            device->gpio_init = NULL;
            device->gpio_sense = nvd0_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nvd0_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nvc0_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_MXM    ] = &nv50_mxm_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv50_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nvc0_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nvc0_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_LTCG   ] = &nvc0_ltcg_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv50_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nvc0_vmmgr_oclass;
//            device->oclass[NVDEV_SUBDEV_BAR    ] = &nvc0_bar_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nvc0_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nve0_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nvc0_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nve0_graph_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nvd0_disp_oclass;
            break;
        case 0xe7:
            device->cname = "GK107";
            device->gpio_init = NULL;
            device->gpio_sense = nvd0_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nvd0_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nvc0_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_MXM    ] = &nv50_mxm_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv50_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nvc0_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nvc0_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_LTCG   ] = &nvc0_ltcg_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv50_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nvc0_vmmgr_oclass;
//            device->oclass[NVDEV_SUBDEV_BAR    ] = &nvc0_bar_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nvc0_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nve0_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nvc0_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nve0_graph_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nvd0_disp_oclass;
            break;
        default:
            nv_fatal(device, "unknown Kepler chipset\n");
            return false;
	}
    
	return true;
}


