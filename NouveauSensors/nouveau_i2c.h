//
//  nouveau_i2c.h
//  HWSensors
//
//  Created by Kozlek on 11.08.12.
//
//

#ifndef __HWSensors__nouveau_i2c__
#define __HWSensors__nouveau_i2c__

#include "nouveau_definitions.h"
#include "i2c_algo_bit.h"
#include "list.h"

#define CONFIG_NOUVEAU_I2C_INTERNAL TRUE
#define CONFIG_NOUVEAU_I2C_NVCLOCK TRUE

#define NV_I2C_PORT(n)    (0x00 + (n))
#define NV_I2C_DEFAULT(n) (0x80 + (n))

struct nouveau_i2c;

struct nouveau_i2c_port {
	struct i2c_adapter adapter;
	struct nouveau_i2c *i2c;
	struct i2c_algo_bit_data bit;
	struct list_head head;
	u8  index;
	u8  type;
	u32 dcb;
	u32 drive;
	u32 sense;
	u32 state;
};

struct nouveau_i2c {
    struct nouveau_device *device;
    
	struct nouveau_i2c_port *(*find)(struct nouveau_i2c *, u8 index);
	int (*identify)(struct nouveau_i2c *, int index,
                    const char *what, struct i2c_board_info *,
                    bool (*match)(struct nouveau_i2c_port *,
                                  struct i2c_board_info *));
	struct list_head ports;
};

struct nouveau_i2c_port *nouveau_i2c_find(struct nouveau_i2c *i2c, u8 index);

void nouveau_i2c_drive_scl(void *, int);
void nouveau_i2c_drive_sda(void *, int);
int  nouveau_i2c_sense_scl(void *);
int  nouveau_i2c_sense_sda(void *);

int  nv_rdi2cr(struct nouveau_i2c_port *, u8 addr, u8 reg);
int  nv_wri2cr(struct nouveau_i2c_port *, u8 addr, u8 reg, u8 val);
bool nv_probe_i2c(struct nouveau_i2c_port *, u8 addr);

int nv_rdaux(struct nouveau_i2c_port *, u32 addr, u8 *data, u8 size);
int nv_wraux(struct nouveau_i2c_port *, u32 addr, u8 *data, u8 size);

extern const struct i2c_algorithm nouveau_i2c_bit_algo;
extern const struct i2c_algorithm nouveau_i2c_aux_algo;

bool nouveau_i2c_create(struct nouveau_device *device);
void nouveau_i2c_probe(struct nouveau_device *device);

#endif /* defined(__HWSensors__nouveau_i2c__) */
