//
//  nouveau_i2c.cpp
//  HWSensors
//
//  Created by Kozlek on 11.08.12.
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

#include "nouveau_i2c.h"

#include "nouveau.h"
#include "vga.h"

#include "nvclock_i2c.h"

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
			&addr,
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

struct nouveau_i2c_port *nouveau_i2c_find(struct nouveau_i2c *i2c, u8 index)
{
    struct nouveau_device *device = i2c->device;
	struct nouveau_i2c_port *port;
    
	if (index == NV_I2C_DEFAULT(0) ||
	    index == NV_I2C_DEFAULT(1)) {
		u8  ver, hdr, cnt, len;
		u16 dcb = nouveau_dcb_i2c_table(device, &ver, &hdr, &cnt, &len);
		if (dcb && ver >= 0x30) {
			u8 auxidx = nv_ro08(device, dcb + 4);
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

#ifdef CONFIG_NOUVEAU_I2C_NVCLOCK
    /* Unlock the extended CRTC registers to get i2c working */
	nvclock_i2c_lock_unlock(device, 0);
    
	/* On NV40 cards the i2c busses can be disabled */
	if(device->card_type == NV_40)
	{
        volatile unsigned char *PCIO = (volatile unsigned char*)(device->mmio->getVirtualAddress() + 0x00601000);
        PCIO[0x3d4] = 0x49;
        PCIO[0x3d5] |= 0x4; /* Unlock the i2c busses */
	}
    
    I2CBusPtr bus = nvclock_i2c_create_bus_ptr(device, STRDUP(what, (int)strlen(what)), port->drive);
#endif
    
    for (i = 0; info[i].addr; i++) {
        
#ifdef CONFIG_NOUVEAU_I2C_NVCLOCK
        info[i].platform_data = (void*)bus;
        
        if(xf86I2CProbeAddress(bus, info[i].addr << 1) && (!match || match(port, &info[i]))) {
#else
        if (nv_probe_i2c(port, info[i].addr) && (!match || match(port, &info[i]))) {
#endif

#ifdef CONFIG_NOUVEAU_I2C_NVCLOCK
            nvclock_i2c_lock_unlock(device, 1);
#endif
            nv_info(device, "found i2c %s: %s\n", what, info[i].type);
			return i;
        }
	}
    
#ifdef CONFIG_NOUVEAU_I2C_NVCLOCK
    nvclock_i2c_lock_unlock(device, 1);
    xf86DestroyI2CBusRec(bus, true, true);
#endif
    
	nv_debug(device, "no i2c devices found.\n");
    
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
	} else if (port->type == DCB_I2C_NV4E_BIT) {
        nv_mask(port->i2c->device, port->drive, 0x2f, state ? 0x21 : 0x01);
    } else if (port->type == DCB_I2C_NVIO_BIT) {
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
	} else if (port->type == DCB_I2C_NV4E_BIT) {
        nv_mask(port->i2c->device, port->drive, 0x1f, state ? 0x11 : 0x01);
    } else if (port->type == DCB_I2C_NVIO_BIT) {
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
	} else if (port->type == DCB_I2C_NV4E_BIT) {
        return !!(nv_rd32(device, port->sense) & 0x00040000);
    } else if (port->type == DCB_I2C_NVIO_BIT) {
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
	} else if (port->type == DCB_I2C_NV4E_BIT) {
        return !!(nv_rd32(device, port->sense) & 0x00080000);
    } else if (port->type == DCB_I2C_NVIO_BIT) {
        if (device->card_type < NV_D0)
            return !!(nv_rd32(device, port->sense) & 0x02);
        else
            return !!(nv_rd32(device, port->sense) & 0x20);
    }
    
	return 0;
}

static const u32 nv50_i2c_port[] = {
	0x00e138, 0x00e150,
    0x00e168, 0x00e180,
	0x00e254, 0x00e274,
    0x00e764, 0x00e780,
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
                    if (port->drive >= ARRAY_SIZE(nv50_i2c_port))
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
                 "nouveau-%d-%x-%x-%x-%d", port->type, info.drive, port->drive, port->sense, i);
        
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
            
			if (CONFIG_NOUVEAU_I2C_INTERNAL) {
                port->adapter.algo = &nouveau_i2c_bit_algo;
                ret = 0; //i2c_add_adapter(&port->adapter);
            } else {
                port->adapter.algo_data = &port->bit;
                port->bit.udelay = 10;
                port->bit.timeout = 2200;
                port->bit.data = port;
                port->bit.setsda = nouveau_i2c_drive_sda;
                port->bit.setscl = nouveau_i2c_drive_scl;
                port->bit.getsda = nouveau_i2c_sense_sda;
                port->bit.getscl = nouveau_i2c_sense_scl;
                ret = i2c_bit_add_bus(&port->adapter);
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

static bool probe_monitoring_device(struct nouveau_i2c_port *i2c, struct i2c_board_info *info)
{
#if CONFIG_NOUVEAU_I2C_NVCLOCK
    nouveau_device *device = i2c->i2c->device;
    
    I2CBusPtr bus = (I2CBusPtr)info->platform_data;
    I2CSlaveAddr addr = info->addr << 1;
    
    I2CDevPtr dev = nvclock_i2c_probe_device(bus, addr, "%1i:%02X", bus, addr);
    
    if (dev) {
        if (!strncmp(info->type, "w83l785ts", sizeof("w83l785ts"))) {
            if(!w83l785r_detect(dev)) return false;
        }
        else if (!strncmp(info->type, "w83781d", sizeof("w83781d"))) {
            if(!w83781d_detect(dev)) return false;
        }
        else if (!strncmp(info->type, "adt7473", sizeof("adt7473"))) {
            if(!adt7473_detect(dev)) return false;
        }
        else if (!strncmp(info->type, "f75375", sizeof("f75375"))) {
            if(!f75375_detect(dev)) return false;
        }
        else if (!strncmp(info->type, "lm99", sizeof("lm99")) ||
                 !strncmp(info->type, "lm90", sizeof("lm90")) ||
                 !strncmp(info->type, "lm63", sizeof("lm63"))) {
            if(!lm99_detect(dev)) return false;
        }
    }
    else return false;

    device->nvclock_i2c_sensor = dev;

    nv_debug(device, "found device: %s\n", device->nvclock_i2c_sensor->chip_name);
        
    switch(device->nvclock_i2c_sensor->chip_id)
    {
        case LM99:
        case MAX6559:
            device->board_temp_get = lm99_get_board_temp;
            device->core_temp_get = lm99_get_gpu_temp;
            break;
        case F75375:
            device->board_temp_get = f75375_get_board_temp;
            device->core_temp_get = f75375_get_gpu_temp;
            device->fan_rpm_get = f75375_get_fanspeed_rpm;
            device->fan_pwm_get = f75375_get_fanspeed_pwm;
            break;
        case W83781D:
            device->board_temp_get = w83781d_get_board_temp;
            device->core_temp_get = w83781d_get_gpu_temp;
            device->fan_rpm_get = w83781d_get_fanspeed_rpm;
            //device->get_i2c_fanspeed_pwm = w83781d_get_fanspeed_pwm;
            break;
        case W83L785R:
            device->board_temp_get = w83l785r_get_board_temp;
            device->core_temp_get = w83l785r_get_gpu_temp;
            device->fan_rpm_get = w83l785r_get_fanspeed_rpm;
            device->fan_pwm_get = w83l785r_get_fanspeed_pwm;
            break;
        case ADT7473:
            device->board_temp_get = adt7473_get_board_temp;
            device->core_temp_get = adt7473_get_gpu_temp;
            device->fan_rpm_get = adt7473_get_fanspeed_rpm;
            device->fan_pwm_get = adt7473_get_fanspeed_pwm;
            break;
        default:
            break;
    }
    
    return true;
#else
	struct i2c_client *client;

	request_module("%s%s", I2C_MODULE_PREFIX, info->type);

	client = i2c_new_device(&i2c->adapter, info);
	if (!client)
		return false;

	if (!client->driver || client->driver->detect(client, info)) {
		i2c_unregister_device(client);
		return false;
	}
    
	return true;
#endif
}
    
    char		type[I2C_NAME_SIZE];
	unsigned short	flags;
	unsigned short	addr;
	void		*platform_data;
	//struct dev_archdata	*archdata;
	//struct device_node *of_node;
	int		irq;

#define I2C_BOARD_INFO(dev_type, dev_addr) \
dev_type, 0, dev_addr, NULL, 0
    
#define I2C_BOARD_INFO_NULL \
"", 0, 0, NULL, 0
    
static struct i2c_board_info
nv_board_infos[] = {
    { I2C_BOARD_INFO("w83l785ts", 0x2d) },
    { I2C_BOARD_INFO("w83781d", 0x2d) },
    { I2C_BOARD_INFO("adt7473", 0x2e) },
    { I2C_BOARD_INFO("adt7473", 0x2d) },
    { I2C_BOARD_INFO("adt7473", 0x2c) },
    { I2C_BOARD_INFO("f75375", 0x2e) },
    { I2C_BOARD_INFO("lm99", 0x4c) },
    { I2C_BOARD_INFO("lm90", 0x4c) },
    { I2C_BOARD_INFO("lm90", 0x4d) },
    { I2C_BOARD_INFO("adm1021", 0x18) },
    { I2C_BOARD_INFO("adm1021", 0x19) },
    { I2C_BOARD_INFO("adm1021", 0x1a) },
    { I2C_BOARD_INFO("adm1021", 0x29) },
    { I2C_BOARD_INFO("adm1021", 0x2a) },
    { I2C_BOARD_INFO("adm1021", 0x2b) },
    { I2C_BOARD_INFO("adm1021", 0x4c) },
    { I2C_BOARD_INFO("adm1021", 0x4d) },
    { I2C_BOARD_INFO("adm1021", 0x4e) },
    { I2C_BOARD_INFO("lm63", 0x18) },
    { I2C_BOARD_INFO("lm63", 0x4e) },
    { }
};

void nouveau_i2c_probe(struct nouveau_device *device)
{
	struct nouveau_i2c *i2c = &device->i2c;
    struct nvbios_extdev_func extdev_entry;
    
	if (!nvbios_extdev_find(device, NVBIOS_EXTDEV_LM89, &extdev_entry)) {
		struct i2c_board_info board[] = {
			{ I2C_BOARD_INFO("lm90", extdev_entry.addr >> 1) },
			{ }
		};
        
		if(i2c->identify(i2c, NV_I2C_DEFAULT(0), "monitoring device", board, probe_monitoring_device) >= 0)
			return;
	}
    
	if (!nvbios_extdev_find(device, NVBIOS_EXTDEV_ADT7473, &extdev_entry)) {
		struct i2c_board_info board[] = {
			{ I2C_BOARD_INFO("adt7473", extdev_entry.addr >> 1) },
			{ }
		};
        
		if(i2c->identify(i2c, NV_I2C_DEFAULT(0), "monitoring device",board, probe_monitoring_device) >= 0)
			return;
	}
    
	/* The vbios doesn't provide the address of an exisiting monitoring
     device. Let's try our static list.
	 */
	if (i2c->identify(i2c, NV_I2C_DEFAULT(0), "monitoring device", nv_board_infos, probe_monitoring_device) < 0)
        i2c->identify(i2c, NV_I2C_DEFAULT(1), "monitoring device", nv_board_infos, probe_monitoring_device);
    
//	struct i2c_board_info info[] = {
//		{ "w83l785ts", 0x0, 0x2d, NULL, 0 },
//		{ "w83781d", 0x0, 0x2d, NULL, 0 },
//		{ "adt7473", 0x0, 0x2e, NULL, 0 },
//		{ "f75375", 0x0, 0x2e, NULL, 0 },
//		{ "lm99", 0x0, 0x4c, NULL, 0 },
//		{ }
//	};
//    
//	i2c->identify(i2c, NV_I2C_DEFAULT(0), "monitoring device", info, probe_monitoring_device);
//    i2c->identify(i2c, NV_I2C_DEFAULT(1), "monitoring device", info, probe_monitoring_device);
}

