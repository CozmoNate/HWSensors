//
//  i2c.h
//  HWSensors
//
//  Created by Kozlek on 31.07.12.
//
//

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

#ifndef HWSensors_i2c_h
#define HWSensors_i2c_h

#include "xf86i2c.h"

#define LM99        0x1
#define MAX6559     0x2
#define F75375      0x4
#define W83L785R    0x8
#define W83781D     0x16
#define ADT7473     0x32

struct nouveau_device;

Bool I2CAddress(I2CDevPtr d, I2CSlaveAddr addr);

void nvclock_i2c_lock_unlock(nouveau_device *device, int lock);
void nvclock_i2c_get_bits(I2CBusPtr b, int *clock, int *data);
void nvclock_i2c_put_bits(I2CBusPtr b, int clock, int data);
void nvclock_nv50_i2c_get_bits(I2CBusPtr bus, int *clock, int *data);
void nvclock_nv50_i2c_put_bits(I2CBusPtr bus, int clock, int data);
I2CBusPtr nvclock_i2c_create_bus_ptr(nouveau_device *device, char *name, int bus);
I2CDevPtr nvclock_i2c_probe_device (I2CBusPtr bus, I2CSlaveAddr addr, const char *format, ...);
void nvclock_i2c_probe_all_devices (I2CBusPtr busses[], int nbus);
I2CDevPtr nvclock_i2c_probe_devices(nouveau_device *device, I2CBusPtr busses[], int num_busses);
bool nvclock_i2c_sensor_init(nouveau_device *device);

/* ADT7473 */
int adt7473_detect(I2CDevPtr dev);
int adt7473_get_board_temp(nouveau_device *device);
int adt7473_get_gpu_temp(nouveau_device *device);
int adt7473_get_fanspeed_rpm(nouveau_device *device);
int adt7473_get_fanspeed_pwm(nouveau_device *device);
int adt7473_set_fanspeed_pwm(I2CDevPtr dev, float speed);
int adt7473_get_fanspeed_mode(I2CDevPtr dev);
void adt7473_set_fanspeed_mode(I2CDevPtr dev, int mode);

/* LM99 */
int lm99_detect(I2CDevPtr dev);
int lm99_get_board_temp(nouveau_device *device);
int lm99_get_gpu_temp(nouveau_device *device);

/* Fintek F75375 */
int f75375_detect(I2CDevPtr dev);
int f75375_get_gpu_temp(nouveau_device *device);
int f75375_get_board_temp(nouveau_device *device);
int f75375_get_fanspeed_rpm(nouveau_device *device);
int f75375_set_fanspeed_rpm(I2CDevPtr dev, int desired_rpm);
int f75375_get_fanspeed_pwm(nouveau_device *device);
int f75375_set_fanspeed_pwm(I2CDevPtr dev, float speed);

/* Winbond W83781D */
int w83781d_detect(I2CDevPtr dev);
int w83781d_get_board_temp(nouveau_device *device);
int w83781d_get_gpu_temp(nouveau_device *device);
int w83781d_get_fanspeed_rpm(nouveau_device *device);
//int w83781d_get_fanspeed_pwm(nouveau_device *device);
//int w83781d_set_fanspeed_pwm(I2CDevPtr dev, float speed);

/* Winbond W83L785R */
int w83l785r_detect(I2CDevPtr dev);
int w83l785r_get_board_temp(nouveau_device *device);
int w83l785r_get_gpu_temp(nouveau_device *device);
int w83l785r_get_fanspeed_rpm(nouveau_device *device);
int w83l785r_get_fanspeed_pwm(nouveau_device *device);
int w83l785r_set_fanspeed_pwm(I2CDevPtr dev, float speed);

#endif
