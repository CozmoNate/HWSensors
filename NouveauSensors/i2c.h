//
//  i2c.h
//  HWSensors
//
//  Created by Kozlek on 31.07.12.
//
//

#ifndef HWSensors_i2c_h
#define HWSensors_i2c_h

#include "nouveau.h"
#include "xf86i2c.h"

#define LM99 0x1
#define MAX6559 0x2
#define F75375 0x4
#define W83L785R 0x8
#define W83781D 0x16
#define ADT7473 0x32

extern NouveauCard *nouveau_card;

Bool I2CAddress(I2CDevPtr d, I2CSlaveAddr addr);

void i2c_lock_unlock(int lock);
void i2c_get_bits(I2CBusPtr b, int *clock, int *data);
void i2c_put_bits(I2CBusPtr b, int clock, int data);
I2CBusPtr i2c_create_bus_ptr(char *name, int bus);
void nv50_i2c_get_bits(I2CBusPtr bus, int *clock, int *data);
void nv50_i2c_put_bits(I2CBusPtr bus, int clock, int data);
I2CBusPtr nv50_i2c_create_bus_ptr(char *name, int bus);
void i2c_probe_device (I2CBusPtr bus, I2CSlaveAddr addr, const char *format, ...);
void i2c_probe_all_devices (I2CBusPtr busses[], int nbus);
I2CDevPtr i2c_probe_devices(I2CBusPtr busses[], int num_busses);


/* ADT7473 */
int adt7473_detect(I2CDevPtr dev);
int adt7473_get_board_temp(I2CDevPtr dev);
int adt7473_get_gpu_temp(I2CDevPtr dev);
int adt7473_get_fanspeed_rpm(I2CDevPtr dev);
float adt7473_get_fanspeed_pwm(I2CDevPtr dev);
int adt7473_set_fanspeed_pwm(I2CDevPtr dev, float speed);
int adt7473_get_fanspeed_mode(I2CDevPtr dev);
void adt7473_set_fanspeed_mode(I2CDevPtr dev, int mode);

/* LM99 */
int lm99_detect(I2CDevPtr dev);
int lm99_get_board_temp(I2CDevPtr dev);
int lm99_get_gpu_temp(I2CDevPtr dev);

/* Fintek F75375 */
int f75375_detect(I2CDevPtr dev);
int f75375_get_gpu_temp(I2CDevPtr dev);
int f75375_get_board_temp(I2CDevPtr dev);
int f75375_get_fanspeed_rpm(I2CDevPtr dev);
int f75375_set_fanspeed_rpm(I2CDevPtr dev, int desired_rpm);
float f75375_get_fanspeed_pwm(I2CDevPtr dev);
int f75375_set_fanspeed_pwm(I2CDevPtr dev, float speed);

/* Winbond W83781D */
int w83781d_detect(I2CDevPtr dev);
int w83781d_get_board_temp(I2CDevPtr dev);
int w83781d_get_gpu_temp(I2CDevPtr dev);
int w83781d_get_fanspeed_rpm(I2CDevPtr dev);
float w83781d_get_fanspeed_pwm(I2CDevPtr dev);
int w83781d_set_fanspeed_pwm(I2CDevPtr dev, float speed);

/* Winbond W83L785R */
int w83l785r_detect(I2CDevPtr dev);
int w83l785r_get_board_temp(I2CDevPtr dev);
int w83l785r_get_gpu_temp(I2CDevPtr dev);
int w83l785r_get_fanspeed_rpm(I2CDevPtr dev);
float w83l785r_get_fanspeed_pwm(I2CDevPtr dev);
int w83l785r_set_fanspeed_pwm(I2CDevPtr dev, float speed);

#endif
