//
//  nouveau_i2c.cpp
//  HWSensors
//
//  Created by Kozlek on 11.08.12.
//
//

#include "nouveau_i2c.h"

#include "nouveau.h"
#include "vga.h"

int nv_rdi2cr(struct nouveau_i2c_port *port, u8 addr, u8 reg)
{
	u8 val;
	struct i2c_msg msgs[] = {
		{
            addr,
            0,
            1,
            &reg
        },
		{
            addr,
            I2C_M_RD,
            1,
            &val
        },
	};
    
	int ret = i2c_transfer(&port->adapter, msgs, 2);
	if (ret != 2)
		return -EIO;
    
	return val;
}

int nv_wri2cr(struct nouveau_i2c_port *port, u8 addr, u8 reg, u8 val)
{
	struct i2c_msg msgs[] = {
		{
            addr,
            0,
            1,
            &reg
        },
		{
            addr,
            0,
            1,
            &val
        },
	};
    
	int ret = i2c_transfer(&port->adapter, msgs, 2);
	if (ret != 2)
		return -EIO;
    
	return 0;
}

bool nv_probe_i2c(struct nouveau_i2c_port *port, u8 addr)
{
	u8 buf = 0;
	struct i2c_msg msgs[] = {
		{
			addr,
			0,
			1,
			&buf,
		},
		{
			addr,
			I2C_M_RD,
			1,
			&buf,
		}
	};
    
	return i2c_transfer(&port->adapter, msgs, 2) == 2;
}

static struct nouveau_i2c_port *nouveau_i2c_find(struct nouveau_i2c *i2c, u8 index)
{
    struct nouveau_device *device = i2c->device;
	struct nouveau_i2c_port *port;
    
	if (index == NV_I2C_DEFAULT(0) ||
	    index == NV_I2C_DEFAULT(1)) {
		u8  ver, hdr, cnt, len;
		u16 pi2c = nouveau_dcb_i2c_table(device, &ver, &hdr, &cnt, &len);
		if (pi2c && ver >= 0x30) {
			u8 auxidx = nv_ro08(device, pi2c + 4);
			if (index != NV_I2C_DEFAULT(0))
				index = (auxidx & 0x0f) >> 0;
			else
				index = (auxidx & 0xf0) >> 4;
		} else {
			index = 2;
		}
	}
    
	list_for_each_entry(port, &i2c->ports, head) {
		if (port->index == index)
			break;
	}

	if (&port->head == &i2c->ports)
		return NULL;
    
	if (device->card_type >= NV_50 && (port->dcb & 0x00000100)) {
		u32 reg = 0x00e500, val;
		if (port->type == 6) {
			reg += port->drive * 0x50;
			val  = 0x2002;
		} else {
			reg += ((port->dcb & 0x1e00) >> 9) * 0x50;
			val  = 0xe001;
		}
        
		/* nfi, but neither auxch or i2c work if it's 1 */
		nv_mask(device, reg + 0x0c, 0x00000001, 0x00000000);
		/* nfi, but switches auxch vs normal i2c */
		nv_mask(device, reg + 0x00, 0x0000f003, val);
	}
    
	return port;
}

static int nouveau_i2c_identify(struct nouveau_i2c *i2c, int index, const char *what, struct i2c_board_info *info, bool (*match)(struct nouveau_i2c_port *, struct i2c_board_info *))
{
    struct nouveau_device *device = i2c->device;
	struct nouveau_i2c_port *port = nouveau_i2c_find(i2c, index);
	int i;
    
	if (!port) {
		nv_debug(device, "no bus when probing %s on %d\n", what, index);
		return -ENODEV;
	}
    
	nv_debug(device, "probing %ss on bus: %d\n", what, port->index);
    
	for (i = 0; info[i].addr; i++) {
		if (nv_probe_i2c(port, info[i].addr) &&
		    (!match || match(port, &info[i]))) {
			nv_info(device, "detected %s: %s\n", what, info[i].type);
			return i;
		}
	}
    
	nv_debug(device, "no devices found.\n");
    
	return -ENODEV;
}

void nouveau_i2c_drive_scl(void *data, int state)
{
	struct nouveau_i2c_port *port = (struct nouveau_i2c_port *)data;
    
	if (port->type == DCB_I2C_NV04_BIT) {
		u8 val = nv_rdvgac(port->i2c, 0, port->drive);
		if (state) val |= 0x20;
		else	   val &= 0xdf;
		nv_wrvgac(port->i2c, 0, port->drive, val | 0x01);
	} else
        if (port->type == DCB_I2C_NV4E_BIT) {
            nv_mask(port->i2c->device, port->drive, 0x2f, state ? 0x21 : 0x01);
        } else
            if (port->type == DCB_I2C_NVIO_BIT) {
                if (state) port->state |= 0x01;
                else	   port->state &= 0xfe;
                nv_wr32(port->i2c->device, port->drive, 4 | port->state);
            }
}

void nouveau_i2c_drive_sda(void *data, int state)
{
	struct nouveau_i2c_port *port = (struct nouveau_i2c_port *)data;
    
	if (port->type == DCB_I2C_NV04_BIT) {
		u8 val = nv_rdvgac(port->i2c, 0, port->drive);
		if (state) val |= 0x10;
		else	   val &= 0xef;
		nv_wrvgac(port->i2c, 0, port->drive, val | 0x01);
	} else
        if (port->type == DCB_I2C_NV4E_BIT) {
            nv_mask(port->i2c->device, port->drive, 0x1f, state ? 0x11 : 0x01);
        } else
            if (port->type == DCB_I2C_NVIO_BIT) {
                if (state) port->state |= 0x02;
                else	   port->state &= 0xfd;
                nv_wr32(port->i2c->device, port->drive, 4 | port->state);
            }
}

int nouveau_i2c_sense_scl(void *data)
{
	struct nouveau_i2c_port *port = (struct nouveau_i2c_port *)data;
	struct nouveau_device *device = port->i2c->device;
    
	if (port->type == DCB_I2C_NV04_BIT) {
		return !!(nv_rdvgac(device, 0, port->sense) & 0x04);
	} else
        if (port->type == DCB_I2C_NV4E_BIT) {
            return !!(nv_rd32(device, port->sense) & 0x00040000);
        } else
            if (port->type == DCB_I2C_NVIO_BIT) {
                if (device->card_type < NV_D0)
                    return !!(nv_rd32(device, port->sense) & 0x01);
                else
                    return !!(nv_rd32(device, port->sense) & 0x10);
            }
    
	return 0;
}

int nouveau_i2c_sense_sda(void *data)
{
	struct nouveau_i2c_port *port = (struct nouveau_i2c_port *)data;
	struct nouveau_device *device = port->i2c->device;
    
	if (port->type == DCB_I2C_NV04_BIT) {
		return !!(nv_rdvgac(device, 0, port->sense) & 0x08);
	} else
        if (port->type == DCB_I2C_NV4E_BIT) {
            return !!(nv_rd32(device, port->sense) & 0x00080000);
        } else
            if (port->type == DCB_I2C_NVIO_BIT) {
                if (device->card_type < NV_D0)
                    return !!(nv_rd32(device, port->sense) & 0x02);
                else
                    return !!(nv_rd32(device, port->sense) & 0x20);
            }
    
	return 0;
}

static const u32 nv50_i2c_port[] = {
	0x00e138, 0x00e150, 0x00e168, 0x00e180,
	0x00e254, 0x00e274, 0x00e764, 0x00e780,
	0x00e79c, 0x00e7b8
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

bool nouveau_i2c_create(struct nouveau_device *device)
{
	struct nouveau_i2c_port *port;
	struct nouveau_i2c *i2c = &device->i2c;
	struct dcb_i2c_entry info;
	int ret, i = -1;
    
    i2c->device = device;
	i2c->find = nouveau_i2c_find;
	i2c->identify = nouveau_i2c_identify;
    
	INIT_LIST_HEAD(&i2c->ports);
    
    nv_debug(device, "parsing i2c dcb table\n");
    
	while (!nouveau_dcb_i2c_parse(device, ++i, &info)) {
		if (info.type == DCB_I2C_UNUSED)
			continue;
        
		port = (struct nouveau_i2c_port *)IOMalloc(sizeof(nouveau_i2c_port));
        
		if (!port) {
			nv_error(device, "failed port memory alloc at %d\n", i);
			break;
		}
        
		port->type = info.type;
		switch (port->type) {
            case DCB_I2C_NV04_BIT:
                port->drive = info.drive;
                port->sense = info.sense;
                break;
            case DCB_I2C_NV4E_BIT:
                port->drive = 0x600800 + info.drive;
                port->sense = port->drive;
                break;
            case DCB_I2C_NVIO_BIT:
                port->drive = info.drive & 0x0f;
                if (device->card_type < NV_D0) {
                    if (info.drive >= ARRAY_SIZE(nv50_i2c_port))
                        break;
                    port->drive = nv50_i2c_port[port->drive];
                    port->sense = port->drive;
                } else {
                    port->drive = 0x00d014 + (port->drive * 0x20);
                    port->sense = port->drive;
                }
                break;
            case DCB_I2C_NVIO_AUX:
                port->drive = info.drive & 0x0f;
                port->sense = port->drive;
                port->adapter.algo = &nouveau_i2c_aux_algo;
                break;
            default:
                break;
		}
        
		if (!port->adapter.algo && !port->drive) {
			nv_error(device, "I2C%d: type %d index %x/%x unknown\n",
                     i, port->type, port->drive, port->sense);
			IOFree(port, sizeof(struct nouveau_i2c_port));
			continue;
		}
        
		snprintf(port->adapter.name, sizeof(port->adapter.name),
                 "nouveau-%x-%x-%d", port->drive, port->sense, i);
        
		//port->adapter.owner = this;
		//port->adapter.dev.parent = &device->pdev->dev;
		port->i2c = i2c;
		port->index = i;
		port->dcb = info.data;
		//i2c_set_adapdata(&port->adapter, i2c);
        
		if (port->adapter.algo != &nouveau_i2c_aux_algo) {
			nouveau_i2c_drive_scl(port, 0);
			nouveau_i2c_drive_sda(port, 1);
			nouveau_i2c_drive_scl(port, 1);
            
			if (CONFIG_NOUVEAU_I2C_INTERNAL_DEFAULT) {
                port->adapter.algo = &nouveau_i2c_bit_algo;
                ret = 0; //i2c_add_adapter(&port->adapter);
            } else {
                port->adapter.algo = &i2c_bit_algo;
                port->adapter.algo_data = &port->bit;
                port->bit.udelay = 10;
                port->bit.timeout = 2200;
                port->bit.data = port;
                port->bit.setsda = nouveau_i2c_drive_sda;
                port->bit.setscl = nouveau_i2c_drive_scl;
                port->bit.getsda = nouveau_i2c_sense_sda;
                port->bit.getscl = nouveau_i2c_sense_scl;
                ret = 0; //i2c_bit_add_bus(&port->adapter);
            }
        } else {
            port->adapter.algo = &nouveau_i2c_aux_algo;
            ret = 0; //i2c_add_adapter(&port->adapter);
        }
        
        if (ret) {
            nv_error(device, "I2C%d: failed register: %d\n", i, ret);
            IOFree(port, sizeof(struct nouveau_i2c_port));
            continue;
        }
        
        nv_debug(device, "adding i2c port %s\n", port->adapter.name);
        
        list_add_tail(&port->head, &i2c->ports);
    }
    
    return 0;
}


