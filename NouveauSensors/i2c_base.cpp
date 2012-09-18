//
//  i2c_base.cpp
//  HWSensors
//
//  Created by Kozlek on 13.08.12.
//
//

#include "i2c_base.h"

/* ----------------------------------------------------
 * the functional interface to the i2c busses.
 * ----------------------------------------------------
 */

/**
 * __i2c_transfer - unlocked flavor of i2c_transfer
 * @adap: Handle to I2C bus
 * @msgs: One or more messages to execute before STOP is issued to
 *	terminate the operation; each message begins with a START.
 * @num: Number of messages to be executed.
 *
 * Returns negative errno, else the number of messages executed.
 *
 * Adapter lock must be held when calling this function. No debug logging
 * takes place. adap->algo->master_xfer existence isn't checked.
 */
int __i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    int ret, try1;
    
	/* Retry automatically on arbitration loss */
    mach_timespec_t end, now;
    
    clock_get_system_nanotime((clock_sec_t*)&end.tv_sec, (clock_usec_t*)&end.tv_nsec);
    
    now.tv_sec = 0;
    now.tv_nsec = adap->timeout * NSEC_PER_USEC;
    
    ADD_MACH_TIMESPEC(&end, &now);
    
	for (ret = 0, try1 = 0; try1 <= adap->retries; try1++) {
		ret = adap->algo->master_xfer(adap, msgs, num);
        
        IOLog("GeforceSensors: _i2c_transfer=%d\n", ret);
        
		if (ret != -EAGAIN)
			break;
        
        clock_get_system_nanotime((clock_sec_t*)&now.tv_sec, (clock_nsec_t*)&now.tv_nsec);
        
		if (CMP_MACH_TIMESPEC(&end, &now) <= 0)
			break;
	}
    
	return ret;
}

/**
 * i2c_transfer - execute a single or combined I2C message
 * @adap: Handle to I2C bus
 * @msgs: One or more messages to execute before STOP is issued to
 *	terminate the operation; each message begins with a START.
 * @num: Number of messages to be executed.
 *
 * Returns negative errno, else the number of messages executed.
 *
 * Note that there is no requirement that each message be sent to
 * the same slave address, although that is the most common model.
 */
int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	int ret;
    
	/* REVISIT the fault reporting model here is weak:
	 *
	 *  - When we get an error after receiving N bytes from a slave,
	 *    there is no way to report "N".
	 *
	 *  - When we get a NAK after transmitting N bytes to a slave,
	 *    there is no way to report "N" ... or to let the master
	 *    continue executing the rest of this combined message, if
	 *    that's the appropriate response.
	 *
	 *  - When for example "num" is two and we successfully complete
	 *    the first message but get an error part way through the
	 *    second, it's unclear whether that should be reported as
	 *    one (discarding status on the second message) or errno
	 *    (discarding status on the first one).
	 */
    
	if (adap->algo->master_xfer) {
		ret = __i2c_transfer(adap, msgs, num);
        
        //IOLog("NouveauI2C: i2c_transfer=%d\n", ret);
        
		return ret;
	} else {
        IOLog("GeforceSensors: I2C level transfers not supported\n");
		return -EOPNOTSUPP;
	}
}
//EXPORT_SYMBOL(i2c_transfer);