//
//  i2c_base.h
//  HWSensors
//
//  Created by Kozlek on 13.08.12.
//
//

#ifndef __HWSensors__i2c_base__
#define __HWSensors__i2c_base__

#include "linux_definitions.h"

struct i2c_msg {
	u16 addr;	/* slave address			*/
	u16 flags;
#define I2C_M_TEN           0x0010	/* this is a ten bit chip address */
#define I2C_M_RD            0x0001	/* read data, from slave to master */
#define I2C_M_STOP          0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
	u16 len;		/* msg length				*/
	u8 *buf;		/* pointer to msg data			*/
};

/* To determine what functionality is present */

#define I2C_FUNC_I2C			0x00000001
#define I2C_FUNC_10BIT_ADDR		0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING	0x00000004 /* I2C_M_IGNORE_NAK etc. */
#define I2C_FUNC_SMBUS_PEC		0x00000008
#define I2C_FUNC_NOSTART		0x00000010 /* I2C_M_NOSTART */
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0x00008000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_QUICK		0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE	0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE	0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA	0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA	0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA	0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA	0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL	0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA	0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK	0x04000000 /* I2C-like block xfer  */
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK	0x08000000 /* w/ 1-byte reg. addr. */

#define I2C_FUNC_SMBUS_BYTE		(I2C_FUNC_SMBUS_READ_BYTE | \
I2C_FUNC_SMBUS_WRITE_BYTE)
#define I2C_FUNC_SMBUS_BYTE_DATA	(I2C_FUNC_SMBUS_READ_BYTE_DATA | \
I2C_FUNC_SMBUS_WRITE_BYTE_DATA)
#define I2C_FUNC_SMBUS_WORD_DATA	(I2C_FUNC_SMBUS_READ_WORD_DATA | \
I2C_FUNC_SMBUS_WRITE_WORD_DATA)
#define I2C_FUNC_SMBUS_BLOCK_DATA	(I2C_FUNC_SMBUS_READ_BLOCK_DATA | \
I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)
#define I2C_FUNC_SMBUS_I2C_BLOCK	(I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)

#define I2C_FUNC_SMBUS_EMUL		(I2C_FUNC_SMBUS_QUICK | \
I2C_FUNC_SMBUS_BYTE | \
I2C_FUNC_SMBUS_BYTE_DATA | \
I2C_FUNC_SMBUS_WORD_DATA | \
I2C_FUNC_SMBUS_PROC_CALL | \
I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
I2C_FUNC_SMBUS_I2C_BLOCK | \
I2C_FUNC_SMBUS_PEC)

/*
 * Data for SMBus Messages
 */
#define I2C_SMBUS_BLOCK_MAX	32	/* As specified in SMBus standard */

struct i2c_adapter;

struct i2c_algorithm {
	/* If an adapter algorithm can't do I2C-level access, set master_xfer
     to NULL. If an adapter algorithm can do SMBus access, set
     smbus_xfer. If set to NULL, the SMBus protocol is simulated
     using common I2C messages */
	/* master_xfer should return the number of messages successfully
     processed, or a negative value on error */
	int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
                       int num);
	int (*smbus_xfer) (struct i2c_adapter *adap, u16 addr,
                       unsigned short flags, char read_write,
                       u8 command, int size, union i2c_smbus_data *data);
    
	/* To determine what the adapter supports */
	u32 (*functionality) (struct i2c_adapter *);
    
    
};

struct i2c_adapter {
	//struct module *owner;
	//unsigned int class;		  /* classes to allow probing for */
	const struct i2c_algorithm *algo; /* the algorithm to access the bus */
	void *algo_data;
    
	/* data fields that are valid for all devices	*/
	//struct rt_mutex bus_lock;
    
	int timeout;			/* in jiffies */
	int retries;
	//struct device dev;		/* the adapter device */
    
	int nr;
	char name[48];
	//struct completion dev_released;
    
	//struct mutex userspace_clients_lock;
	//struct list_head userspace_clients;
};

struct i2c_board_info {
	char		type[I2C_NAME_SIZE];
	unsigned short	flags;
	unsigned short	addr;
	void		*platform_data;
	//struct dev_archdata	*archdata;
	//struct device_node *of_node;
	int		irq;
};

bool __i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);
bool i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);

#endif /* defined(__HWSensors__i2c_base__) */
