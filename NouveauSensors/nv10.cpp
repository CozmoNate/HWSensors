//
//  nv10.cpp
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

/*
 * Copyright (C) 2009 Francisco Jerez.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "nv10.h"

#include "nouveau_definitions.h"
#include "nouveau.h"

void nv10_gpio_init(struct nouveau_device *device)
{
	nv_wr32(device, 0x001140, 0x00000000);
	nv_wr32(device, 0x001100, 0xffffffff);
	nv_wr32(device, 0x001144, 0x00000000);
	nv_wr32(device, 0x001104, 0xffffffff);
}

int nv10_gpio_sense(struct nouveau_device *device, int line)
{
	if (line < 2) {
		line = line * 16;
		line = nv_rd32(device, 0x00600818) >> line;
		return !!(line & 0x0100);
	} else
        if (line < 10) {
            line = (line - 2) * 4;
            line = nv_rd32(device, 0x0060081c) >> line;
            return !!(line & 0x04);
        } else
            if (line < 14) {
                line = (line - 10) * 4;
                line = nv_rd32(device, 0x00600850) >> line;
                return !!(line & 0x04);
            }
    
	return false;
}
