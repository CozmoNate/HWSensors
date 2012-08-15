//
//  nouveau_i2c_bit.cpp
//  HWSensors
//
//  Created by Kozlek on 13.08.12.
//
//

#include "nouveau_i2c.h"
#include "nouveau.h"

#ifdef CONFIG_NOUVEAU_I2C_INTERNAL
#define T_TIMEOUT  2200000
#define T_RISEFALL 1000
#define T_HOLD     5000

static inline void
i2c_drive_scl(struct nouveau_i2c_port *port, int state)
{
	nouveau_i2c_drive_scl(port, state);
}

static inline void
i2c_drive_sda(struct nouveau_i2c_port *port, int state)
{
	nouveau_i2c_drive_sda(port, state);
}

static inline int
i2c_sense_scl(struct nouveau_i2c_port *port)
{
	return nouveau_i2c_sense_scl(port);
}

static inline int
i2c_sense_sda(struct nouveau_i2c_port *port)
{
	return nouveau_i2c_sense_sda(port);
}

static void
i2c_delay(struct nouveau_i2c_port *port, u32 nsec)
{
	//IODelay((nsec + 500) / 1000);
    IOPause(nsec + 500);
}

static bool
i2c_raise_scl(struct nouveau_i2c_port *port)
{
	u32 timeout = T_TIMEOUT / T_RISEFALL;
    
	i2c_drive_scl(port, 1);
	do {
		i2c_delay(port, T_RISEFALL);
	} while (!i2c_sense_scl(port) && --timeout);
    
	return timeout != 0;
}

static int
i2c_start(struct nouveau_i2c_port *port)
{
	int ret = 0;
    
	port->state  = i2c_sense_scl(port);
	port->state |= i2c_sense_sda(port) << 1;
	if (port->state != 3) {
		i2c_drive_scl(port, 0);
		i2c_drive_sda(port, 1);
		if (!i2c_raise_scl(port))
			ret = -EBUSY;
	}
    
	i2c_drive_sda(port, 0);
	i2c_delay(port, T_HOLD);
	i2c_drive_scl(port, 0);
	i2c_delay(port, T_HOLD);
    
    nv_trace(port->i2c->device, "i2c_start=%d\n", ret);
    
	return ret;
}

static void
i2c_stop(struct nouveau_i2c_port *port)
{
	i2c_drive_scl(port, 0);
	i2c_drive_sda(port, 0);
	i2c_delay(port, T_RISEFALL);
    
	i2c_drive_scl(port, 1);
	i2c_delay(port, T_HOLD);
	i2c_drive_sda(port, 1);
	i2c_delay(port, T_HOLD);
}

static int
i2c_bitw(struct nouveau_i2c_port *port, int sda)
{
	i2c_drive_sda(port, sda);
	i2c_delay(port, T_RISEFALL);
    
	if (!i2c_raise_scl(port))
		return -ETIMEDOUT;
	i2c_delay(port, T_HOLD);
    
	i2c_drive_scl(port, 0);
	i2c_delay(port, T_HOLD);
	return 0;
}

static int
i2c_bitr(struct nouveau_i2c_port *port)
{
	int sda;
    
	i2c_drive_sda(port, 1);
	i2c_delay(port, T_RISEFALL);
    
	if (!i2c_raise_scl(port))
		return -ETIMEDOUT;
	i2c_delay(port, T_HOLD);
    
	sda = i2c_sense_sda(port);
    
	i2c_drive_scl(port, 0);
	i2c_delay(port, T_HOLD);
	return sda;
}

static int
i2c_get_byte(struct nouveau_i2c_port *port, u8 *byte, bool last)
{
	int i, bit, ret;
    
	*byte = 0;
	for (i = 7; i >= 0; i--) {
		bit = i2c_bitr(port);
		if (bit < 0)
			return bit;
		*byte |= bit << i;
	}
    
    ret = i2c_bitw(port, last ? 1 : 0);
    
    nv_trace(port->i2c->device, "i2c_get_byte=%d\n", ret);
    
	return ret;
}

static int
i2c_put_byte(struct nouveau_i2c_port *port, u8 byte)
{
	int i, ret;
	for (i = 7; i >= 0; i--) {
		ret = i2c_bitw(port, !!(byte & (1 << i)));
		if (ret < 0)
			return ret;
	}
    
	ret = i2c_bitr(port);
	if (ret == 1) /* nack */
		ret = -EIO;
    
    nv_trace(port->i2c->device, "i2c_put_byte=%d\n", ret);
    
	return ret;
}

static int
i2c_addr(struct nouveau_i2c_port *port, struct i2c_msg *msg)
{
	u32 addr = msg->addr << 1;
	if (msg->flags & I2C_M_RD)
		addr |= 1;
	return i2c_put_byte(port, addr);
}

static int
i2c_bit_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct nouveau_i2c_port *port = (struct nouveau_i2c_port *)adap;
	struct i2c_msg *msg = msgs;
	int ret = 0, mcnt = num;
    
	while (!ret && mcnt--) {
		u8 remaining = msg->len;
		u8 *ptr = msg->buf;
        
		ret = i2c_start(port);
		if (ret == 0)
			ret = i2c_addr(port, msg);
        
		if (msg->flags & I2C_M_RD) {
			while (!ret && remaining--)
				ret = i2c_get_byte(port, ptr++, !remaining);
		} else {
			while (!ret && remaining--)
				ret = i2c_put_byte(port, *ptr++);
		}
        
		msg++;
	}
    
	i2c_stop(port);
	return (ret < 0) ? ret : num;
}
#else
static int
i2c_bit_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	return -ENODEV;
}
#endif

static u32
i2c_bit_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

const struct i2c_algorithm nouveau_i2c_bit_algo = {i2c_bit_xfer, NULL, i2c_bit_func};
