//
//  nv40.c
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
#include "nv40.h"
#include "nv10.h"

bool nv40_identify(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0x40:
            device->cname = "NV40";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
           
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x41:
            device->cname = "NV41";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x42:
            device->cname = "NV42";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x43:
            device->cname = "NV43";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x45:
            device->cname = "NV45";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x47:
            device->cname = "G70";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x49:
            device->cname = "G71";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x4b:
            device->cname = "G73";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv04_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x44:
            device->cname = "NV44";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x46:
            device->cname = "G72";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x4a:
            device->cname = "NV44A";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x4c:
            device->cname = "C61";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x4e:
            device->cname = "C51";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x63:
            device->cname = "C73";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x67:
            device->cname = "C67";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        case 0x68:
            device->cname = "C68";
            device->gpio_init = nv10_gpio_init;
            device->gpio_sense = nv10_gpio_sense;
            
//            device->oclass[NVDEV_SUBDEV_VBIOS  ] = &nouveau_bios_oclass;
//            device->oclass[NVDEV_SUBDEV_GPIO   ] = &nv10_gpio_oclass;
//            device->oclass[NVDEV_SUBDEV_I2C    ] = &nouveau_i2c_oclass;
//            device->oclass[NVDEV_SUBDEV_CLOCK  ] = &nv40_clock_oclass;
//            device->oclass[NVDEV_SUBDEV_DEVINIT] = &nv1a_devinit_oclass;
//            device->oclass[NVDEV_SUBDEV_MC     ] = &nv44_mc_oclass;
//            device->oclass[NVDEV_SUBDEV_TIMER  ] = &nv04_timer_oclass;
//            device->oclass[NVDEV_SUBDEV_FB     ] = &nv40_fb_oclass;
//            device->oclass[NVDEV_SUBDEV_INSTMEM] = &nv40_instmem_oclass;
//            device->oclass[NVDEV_SUBDEV_VM     ] = &nv04_vmmgr_oclass;
//            device->oclass[NVDEV_ENGINE_DMAOBJ ] = &nv04_dmaeng_oclass;
//            device->oclass[NVDEV_ENGINE_FIFO   ] = &nv40_fifo_oclass;
//            device->oclass[NVDEV_ENGINE_SW     ] = &nv10_software_oclass;
//            device->oclass[NVDEV_ENGINE_GR     ] = &nv40_graph_oclass;
//            device->oclass[NVDEV_ENGINE_MPEG   ] = &nv40_mpeg_oclass;
//            device->oclass[NVDEV_ENGINE_DISP   ] = &nv04_disp_oclass;
            break;
        default:
            nv_fatal(device, "unknown Curie chipset\n");
            return false;
	}
    
	return true;
}

static u32 read_pll_1(struct nouveau_device *device, u32 reg)
{
	u32 ctrl = nv_rd32(device, reg + 0x00);
	int P = (ctrl & 0x00070000) >> 16;
	int N = (ctrl & 0x0000ff00) >> 8;
	int M = (ctrl & 0x000000ff) >> 0;
	u32 ref = 27000, clk = 0;
    
	if (ctrl & 0x80000000)
		clk = ref * N / M;
    
	return clk >> P;
}

static u32 read_pll_2(struct nouveau_device *device, u32 reg)
{
	u32 ctrl = nv_rd32(device, reg + 0x00);
	u32 coef = nv_rd32(device, reg + 0x04);
	int N2 = (coef & 0xff000000) >> 24;
	int M2 = (coef & 0x00ff0000) >> 16;
	int N1 = (coef & 0x0000ff00) >> 8;
	int M1 = (coef & 0x000000ff) >> 0;
	int P = (ctrl & 0x00070000) >> 16;
	u32 ref = 27000, clk = 0;
    
	if ((ctrl & 0x80000000) && M1) {
		clk = ref * N1 / M1;
		if ((ctrl & 0x40000100) == 0x40000000) {
			if (M2)
				clk = clk * N2 / M2;
			else
				clk = 0;
		}
	}
    
	return clk >> P;
}

static u32 read_clk(struct nouveau_device *device, u32 src)
{
	switch (src) {
        case 3:
            return read_pll_2(device, 0x004000);
        case 2:
            return read_pll_1(device, 0x004008);
        default:
            break;
	}
    
	return 0;
}

int nv40_pm_clocks_get(struct nouveau_device *device, u8 source)
{
	u32 ctrl = nv_rd32(device, 0x00c040);
    
    switch (source) {
        case nouveau_clock_core:
            return read_clk(device, (ctrl & 0x00000003) >> 0);
            
        case nouveau_clock_shader:
            return read_clk(device, (ctrl & 0x00000030) >> 4);
            
        case  nouveau_clock_memory:
            return read_pll_2(device, 0x4020);
            
        default:
            return 0;
    }
}

bool nv40_pm_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty)
{
	if (line == 2) {
		u32 reg = nv_rd32(device, 0x0010f0);
		if (reg & 0x80000000) {
			*duty = (reg & 0x7fff0000) >> 16;
			*divs = (reg & 0x00007fff);
			return true;
		}
	} else
        if (line == 9) {
            u32 reg = nv_rd32(device, 0x0015f4);
            if (reg & 0x80000000) {
                *divs = nv_rd32(device, 0x0015f8);
                *duty = (reg & 0x7fffffff);
                return true;
            }
        } else {
            nv_error(device, "unknown pwm ctrl for gpio %d\n", line);
            return false;
        }
    
	return false;
}
