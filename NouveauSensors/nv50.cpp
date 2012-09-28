//
//  nv50.c
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
#include "nva3.h"
#include "nouveau_therm.h"

bool nv50_identify(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0x50:
            device->cname = "G80";
            break;
        case 0x84:
            device->cname = "G84";
            break;
        case 0x86:
            device->cname = "G86";
            break;
        case 0x92:
            device->cname = "G92";
            break;
        case 0x94:
            device->cname = "G94";
            break;
        case 0x96:
            device->cname = "G96";
            break;
        case 0x98:
            device->cname = "G98";
            break;
        case 0xa0:
            device->cname = "G200";
            break;
        case 0xaa:
            device->cname = "MCP77/MCP78";
            break;
        case 0xac:
            device->cname = "MCP79/MCP7A";
            break;
        case 0xa3:
            device->cname = "GT215";
            break;
        case 0xa5:
            device->cname = "GT216";
            break;
        case 0xa8:
            device->cname = "GT218";
            break;
        case 0xaf:
            device->cname = "MCP89";
            break;
        default:
            nv_fatal(device, "unknown Tesla chipset\n");
            return false;
	}
    
	return true;
}

void nv50_init(struct nouveau_device *device)
{
    switch (device->chipset) {
        case 0xa3:
        case 0xa5:
        case 0xa8:
        case 0xaf:
            device->clocks_get = nva3_clocks_get;
            break;
        default:
            device->clocks_get = nv50_clocks_get;
            break;
    }
    
    device->gpio_sense = nv50_gpio_sense;
    device->gpio_find = nouveau_gpio_find;
    device->gpio_get = nouveau_gpio_get;
    
    device->temp_get = nv50_temp_get;
    device->pwm_get = nv50_fan_pwm_get;
    device->fan_pwm_get = nouveau_therm_fan_get;
    device->fan_rpm_get = nouveau_therm_fan_sense;
}

enum clk_src {
	clk_src_crystal,
	clk_src_href,
	clk_src_hclk,
	clk_src_hclkm3,
	clk_src_hclkm3d2,
	clk_src_host,
	clk_src_nvclk,
	clk_src_sclk,
	clk_src_mclk,
	clk_src_vdec,
	clk_src_dom6
};

static u32 read_clk(struct nouveau_device *, enum clk_src);

static u32 read_div(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0x50: /* it exists, but only has bit 31, not the dividers.. */
        case 0x84:
        case 0x86:
        case 0x98:
        case 0xa0:
            return nv_rd32(device, 0x004700);
        case 0x92:
        case 0x94:
        case 0x96:
            return nv_rd32(device, 0x004800);
        default:
            return 0x00000000;
	}
}

static u32 read_pll_src(struct nouveau_device *device, u32 base)
{
	u32 coef, ref = read_clk(device, clk_src_crystal);
	u32 rsel = nv_rd32(device, 0x00e18c);
	int P, N, M = 0, id = 0;
    
	switch (device->chipset) {
        case 0x50:
        case 0xa0:
            switch (base) {
                case 0x4020:
                case 0x4028: id = !!(rsel & 0x00000004); break;
                case 0x4008: id = !!(rsel & 0x00000008); break;
                case 0x4030: id = 0; break;
                default:
                    nv_error(device, "ref: bad pll 0x%06x\n", base);
                    return 0;
            }
            
            coef = nv_rd32(device, 0x00e81c + (id * 0x0c));
            ref *=  (coef & 0x01000000) ? 2 : 4;
            P    =  (coef & 0x00070000) >> 16;
            N    = ((coef & 0x0000ff00) >> 8) + 1;
            M    = ((coef & 0x000000ff) >> 0) + 1;
            break;
        case 0x84:
        case 0x86:
        case 0x92:
            coef = nv_rd32(device, 0x00e81c);
            P    = (coef & 0x00070000) >> 16;
            N    = (coef & 0x0000ff00) >> 8;
            M    = (coef & 0x000000ff) >> 0;
            break;
        case 0x94:
        case 0x96:
        case 0x98:
            rsel = nv_rd32(device, 0x00c050);
            switch (base) {
                case 0x4020: rsel = (rsel & 0x00000003) >> 0; break;
                case 0x4008: rsel = (rsel & 0x0000000c) >> 2; break;
                case 0x4028: rsel = (rsel & 0x00001800) >> 11; break;
                case 0x4030: rsel = 3; break;
                default:
                    nv_error(device, "ref: bad pll 0x%06x\n", base);
                    return 0;
            }
            
            switch (rsel) {
                case 0: id = 1; break;
                case 1: return read_clk(device, clk_src_crystal);
                case 2: return read_clk(device, clk_src_href);
                case 3: id = 0; break;
            }
            
            coef =  nv_rd32(device, 0x00e81c + (id * 0x28));
            P    = (nv_rd32(device, 0x00e824 + (id * 0x28)) >> 16) & 7;
            P   += (coef & 0x00070000) >> 16;
            N    = (coef & 0x0000ff00) >> 8;
            M    = (coef & 0x000000ff) >> 0;
            break;
        default:
            //BUG_ON(1);
            break;
	}
    
	if (M)
		return (ref * N / M) >> P;
	return 0;
}

static u32 read_pll_ref(struct nouveau_device *device, u32 base)
{
	u32 src, mast = nv_rd32(device, 0x00c040);
    
	switch (base) {
        case 0x004028:
            src = !!(mast & 0x00200000);
            break;
        case 0x004020:
            src = !!(mast & 0x00400000);
            break;
        case 0x004008:
            src = !!(mast & 0x00010000);
            break;
        case 0x004030:
            src = !!(mast & 0x02000000);
            break;
        case 0x00e810:
            return read_clk(device, clk_src_crystal);
        default:
            nv_error(device, "bad pll 0x%06x\n", base);
            return 0;
	}
    
	if (src)
		return read_clk(device, clk_src_href);
	return read_pll_src(device, base);
}

static u32 read_pll(struct nouveau_device *device, u32 base)
{
	u32 mast = nv_rd32(device, 0x00c040);
	u32 ctrl = nv_rd32(device, base + 0);
	u32 coef = nv_rd32(device, base + 4);
	u32 ref = read_pll_ref(device, base);
	u32 clk = 0;
	int N1, N2, M1, M2;
    
	if (base == 0x004028 && (mast & 0x00100000)) {
		/* wtf, appears to only disable post-divider on nva0 */
		if (device->chipset != 0xa0)
			return read_clk(device, clk_src_dom6);
	}
    
	N2 = (coef & 0xff000000) >> 24;
	M2 = (coef & 0x00ff0000) >> 16;
	N1 = (coef & 0x0000ff00) >> 8;
	M1 = (coef & 0x000000ff);
	if ((ctrl & 0x80000000) && M1) {
		clk = ref * N1 / M1;
		if ((ctrl & 0x40000100) == 0x40000000) {
			if (M2)
				clk = clk * N2 / M2;
			else
				clk = 0;
		}
	}
    
	return clk;
}

static u32 read_clk(struct nouveau_device *device, enum clk_src src)
{
	u32 mast = nv_rd32(device, 0x00c040);
	u32 P = 0;
    
	switch (src) {
        case clk_src_crystal:
            return device->crystal;
        case clk_src_href:
            return 100000; /* PCIE reference clock */
        case clk_src_hclk:
            return read_clk(device, clk_src_href) * 27778 / 10000;
        case clk_src_hclkm3:
            return read_clk(device, clk_src_hclk) * 3;
        case clk_src_hclkm3d2:
            return read_clk(device, clk_src_hclk) * 3 / 2;
        case clk_src_host:
            switch (mast & 0x30000000) {
                case 0x00000000: return read_clk(device, clk_src_href);
                case 0x10000000: break;
                case 0x20000000: /* !0x50 */
                case 0x30000000: return read_clk(device, clk_src_hclk);
            }
            break;
        case clk_src_nvclk:
            if (!(mast & 0x00100000))
                P = (nv_rd32(device, 0x004028) & 0x00070000) >> 16;
            switch (mast & 0x00000003) {
                case 0x00000000: return read_clk(device, clk_src_crystal) >> P;
                case 0x00000001: return read_clk(device, clk_src_dom6);
                case 0x00000002: return read_pll(device, 0x004020) >> P;
                case 0x00000003: return read_pll(device, 0x004028) >> P;
            }
            break;
        case clk_src_sclk:
            P = (nv_rd32(device, 0x004020) & 0x00070000) >> 16;
            switch (mast & 0x00000030) {
                case 0x00000000:
                    if (mast & 0x00000080)
                        return read_clk(device, clk_src_host) >> P;
                    return read_clk(device, clk_src_crystal) >> P;
                case 0x00000010: break;
                case 0x00000020: return read_pll(device, 0x004028) >> P;
                case 0x00000030: return read_pll(device, 0x004020) >> P;
            }
            break;
        case clk_src_mclk:
            P = (nv_rd32(device, 0x004008) & 0x00070000) >> 16;
            if (nv_rd32(device, 0x004008) & 0x00000200) {
                switch (mast & 0x0000c000) {
                    case 0x00000000:
                        return read_clk(device, clk_src_crystal) >> P;
                    case 0x00008000:
                    case 0x0000c000:
                        return read_clk(device, clk_src_href) >> P;
                }
            } else {
                return read_pll(device, 0x004008) >> P;
            }
            break;
        case clk_src_vdec:
            P = (read_div(device) & 0x00000700) >> 8;
            switch (device->chipset) {
                case 0x84:
                case 0x86:
                case 0x92:
                case 0x94:
                case 0x96:
                case 0xa0:
                    switch (mast & 0x00000c00) {
                        case 0x00000000:
                            if (device->chipset == 0xa0) /* wtf?? */
                                return read_clk(device, clk_src_nvclk) >> P;
                            return read_clk(device, clk_src_crystal) >> P;
                        case 0x00000400:
                            return 0;
                        case 0x00000800:
                            if (mast & 0x01000000)
                                return read_pll(device, 0x004028) >> P;
                            return read_pll(device, 0x004030) >> P;
                        case 0x00000c00:
                            return read_clk(device, clk_src_nvclk) >> P;
                    }
                    break;
                case 0x98:
                    switch (mast & 0x00000c00) {
                        case 0x00000000:
                            return read_clk(device, clk_src_nvclk) >> P;
                        case 0x00000400:
                            return 0;
                        case 0x00000800:
                            return read_clk(device, clk_src_hclkm3d2) >> P;
                        case 0x00000c00:
                            return read_clk(device, clk_src_mclk) >> P;
                    }
                    break;
            }
            break;
        case clk_src_dom6:
            switch (device->chipset) {
                case 0x50:
                case 0xa0:
                    return read_pll(device, 0x00e810) >> 2;
                case 0x84:
                case 0x86:
                case 0x92:
                case 0x94:
                case 0x96:
                case 0x98:
                    P = (read_div(device) & 0x00000007) >> 0;
                    switch (mast & 0x0c000000) {
                        case 0x00000000: return read_clk(device, clk_src_href);
                        case 0x04000000: break;
                        case 0x08000000: return read_clk(device, clk_src_hclk);
                        case 0x0c000000:
                            return read_clk(device, clk_src_hclkm3) >> P;
                    }
                    break;
                default:
                    break;
            }
        default:
            break;
	}
    
	nv_debug(device, "unknown clock source %d 0x%08x\n", src, mast);
	return 0;
}

int nv50_clocks_get(struct nouveau_device *device, u8 source)
{
	if (device->chipset == 0xaa ||
	    device->chipset == 0xac)
		return 0;
    
    switch (source) {
        case nouveau_clock_core:
            return read_clk(device, clk_src_nvclk);
            
        case nouveau_clock_shader:
            return read_clk(device, clk_src_sclk);
            
        case  nouveau_clock_memory:
            return read_clk(device, clk_src_mclk);

        default:
            if (device->chipset != 0x50) {
                switch (source) {
                    case nouveau_clock_vdec:
                        return read_clk(device, clk_src_vdec);
                        
                    case nouveau_clock_dom6:
                        return read_clk(device, clk_src_dom6);
                        
                    default:
                        return 0;
                }
            }
            
            return 0;
    }
    
	return 0;
}

static bool nv50_gpio_location(int line, u32 *reg, u32 *shift)
{
	const u32 nv50_gpio_reg[4] = { 0xe104, 0xe108, 0xe280, 0xe284 };
    
	if (line >= 32)
		return false;
    
	*reg = nv50_gpio_reg[line >> 3];
	*shift = (line & 7) << 2;
    
	return true;
}

int nv50_gpio_sense(struct nouveau_device *device, int line)
{
	u32 reg, shift;
    
	if (!nv50_gpio_location(line, &reg, &shift))
		return -EINVAL;
    
	return !!(nv_rd32(device, reg) & (4 << shift));
}

int nv50_temp_get(struct nouveau_device *device)
{
	return nv_rd32(device, 0x20400);
}

static int pwm_info(struct nouveau_device *device, int *line, int *ctrl, int *indx)
{
	if (*line == 0x04) {
		*ctrl = 0x00e100;
		*line = 4;
		*indx = 0;
	} else
        if (*line == 0x09) {
            *ctrl = 0x00e100;
            *line = 9;
            *indx = 1;
        } else
            if (*line == 0x10) {
                *ctrl = 0x00e28c;
                *line = 0;
                *indx = 0;
            } else {
                nv_error(device, "unknown pwm ctrl for gpio %d\n", *line);
                return -ENODEV;
            }
    
	return 0;
}

int nv50_fan_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty)
{
	int ctrl, id, ret = pwm_info(device, &line, &ctrl, &id);
	if (ret)
		return ret;
    
	if (nv_rd32(device, ctrl) & (1 << line)) {
		*divs = nv_rd32(device, 0x00e114 + (id * 8));
		*duty = nv_rd32(device, 0x00e118 + (id * 8));
		return 0;
	}
    
	return -ENODEV;
}


