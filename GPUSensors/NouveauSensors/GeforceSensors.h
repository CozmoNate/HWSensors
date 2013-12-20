/*
 *  GeforceSensors.h
 *  HWSensors
 *
 *  Created by kozlek on 19/04/12.
 *  Copyright 2010 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 */

/*
 * Copyright 2007-2008 Nouveau Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <IOKit/IOService.h>
#include "nouveau.h"
#include "xf86i2c.h"

#include "GPUSensors.h"

class GeforceSensors : public GPUSensors
{
    OSDeclareDefaultStructors(GeforceSensors)    
	
private:
    nouveau_device      card;
    
    I2CDevPtr           i2c_sensor;
    int                 i2c_get_board_temp(I2CDevPtr dev);
    int                 i2c_get_gpu_temp(I2CDevPtr dev);
    int                 i2c_get_fanspeed_rpm(I2CDevPtr dev);
    float               i2c_get_fanspeed_pwm(I2CDevPtr dev);
    int                 i2c_get_fanspeed_mode(I2CDevPtr dev);
    
protected:
    virtual bool        getSensorValue(FakeSMCSensor *sensor, float* value);
    virtual bool        shouldWaitForAccelerator();
    virtual bool        acceleratorLoadedCheck();
    virtual bool        managedStart(IOService *provider);
    
public:
    virtual void        stop(IOService * provider);
};