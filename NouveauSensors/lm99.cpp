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
 *
 * LM99 hardware monitoring
 */

#include "nouveau_definitions.h"

#include "nvclock_i2c.h"

/* various defines for register offsets and such are needed */

#define LM99_REG_LOCAL_TEMP     0x00
#define LM99_REG_REMOTE_TEMP    0x01
#define LM99_REG_MAN_ID         0xfe
#define NATSEM_MAN_ID           0x01
#define MAXIM_MAN_ID            0x4d
#define MAX6659_REG_R_REMOTE_EMERG	0x16
#define MAX6696_REG_R_STATUS2		0x12
#define LM99_REG_CHIP_ID        0xff
#define LM90_REG_R_CONFIG1      0x03
#define LM90_REG_R_CONFIG2		0xBF
#define LM90_REG_R_CONVRATE		0x04

/* This function should return the chip type .. */
int lm99_detect(I2CDevPtr dev)
{
	I2CByte man_id, chip_id, config1, config2, convrate, address = dev->SlaveAddr / 2;
    const char *name = NULL;
    
	xf86I2CReadByte  (dev, LM99_REG_MAN_ID, &man_id);
	xf86I2CReadByte  (dev, LM99_REG_CHIP_ID, &chip_id);
    xf86I2CReadByte  (dev, LM90_REG_R_CONFIG1, &config1);
    xf86I2CReadByte  (dev, LM90_REG_R_CONVRATE, &convrate);
    
    if (man_id < 0 || chip_id < 0 || config1 < 0 || convrate < 0)
		return 0;
    
    if (man_id == 0x01 || man_id == 0x5C || man_id == 0x41) {
        xf86I2CReadByte  (dev, LM90_REG_R_CONVRATE, &config2);
		if (config2 < 0)
			return 0;
	} else config2 = 0;
    
    if ((address == 0x4C || address == 0x4D) && man_id == 0x01) { /* National Semiconductor */
		if ((config1 & 0x2A) == 0x00 && (config2 & 0xF8) == 0x00 && convrate <= 0x09) {
			if (address == 0x4C & (chip_id & 0xF0) == 0x20) { /* LM90 */
				name = "National Semiconductor LM90";
                dev->chip_id = LM99;
			} else if ((chip_id & 0xF0) == 0x30) { /* LM89/LM99 */
                name = "National Semiconductor :M99";
                dev->chip_id = LM99;
            } else if (address == 0x4C && (chip_id & 0xF0) == 0x10) { /* LM86 */
                name = "National Semiconductor LM86";
                dev->chip_id = LM99;
            }
		}
	}
    else if ((address == 0x4C || address == 0x4D) && man_id == 0x41) { /* Analog Devices */
        if ((chip_id & 0xF0) == 0x40 /* ADM1032 */ && (config1 & 0x3F) == 0x00 && convrate <= 0x0A) {
            name = "Analog Devices ADM1032";
            dev->chip_id = LM99;
            /*
             * The ADM1032 supports PEC, but only if combined
             * transactions are not used.
             */
            //                if (i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
            //                    info->flags |= I2C_CLIENT_PEC;
        } else if (chip_id == 0x51 /* ADT7461 */ && (config1 & 0x1B) == 0x00 && convrate <= 0x0A) {
            name = "Analog Devices ADT7461";
            dev->chip_id = LM99;
        } else if (chip_id == 0x57 /* ADT7461A, NCT1008 */ && (config1 & 0x1B) == 0x00 && convrate <= 0x0A) {
            name = "Analog Devices ADT7461A";
            dev->chip_id = MAX6559;
        }
    } else if (man_id == 0x4D) { /* Maxim */
        I2CByte emerg, emerg2, status2;
        
        /*
         * We read MAX6659_REG_R_REMOTE_EMERG twice, and re-read
         * LM90_REG_R_MAN_ID in between. If MAX6659_REG_R_REMOTE_EMERG
         * exists, both readings will reflect the same value. Otherwise,
         * the readings will be different.
         */
        xf86I2CReadByte  (dev, MAX6659_REG_R_REMOTE_EMERG, &emerg);
        xf86I2CReadByte  (dev, LM99_REG_MAN_ID, &man_id);
        xf86I2CReadByte  (dev, MAX6659_REG_R_REMOTE_EMERG, &emerg2);
        xf86I2CReadByte  (dev, MAX6696_REG_R_STATUS2, &status2);
        
        if (emerg < 0 || man_id < 0 || emerg2 < 0 || status2 < 0)
            return 0;
        
        /*
         * The MAX6657, MAX6658 and MAX6659 do NOT have a chip_id
         * register. Reading from that address will return the last
         * read value, which in our case is those of the man_id
         * register. Likewise, the config1 register seems to lack a
         * low nibble, so the value will be those of the previous
         * read, so in our case those of the man_id register.
         * MAX6659 has a third set of upper temperature limit registers.
         * Those registers also return values on MAX6657 and MAX6658,
         * thus the only way to detect MAX6659 is by its address.
         * For this reason it will be mis-detected as MAX6657 if its
         * address is 0x4C.
         */
        if (chip_id == man_id && (address == 0x4C || address == 0x4D || address == 0x4E) && (config1 & 0x1F) == (man_id & 0x0F) && convrate <= 0x09) {
            if (address == 0x4C)
                name = "Maxim MAX6657";
            else
                name = "Maxim MAX6659";
            dev->chip_id = MAX6559;
        }
        /*
         * Even though MAX6695 and MAX6696 do not have a chip ID
         * register, reading it returns 0x01. Bit 4 of the config1
         * register is unused and should return zero when read. Bit 0 of
         * the status2 register is unused and should return zero when
         * read.
         *
         * MAX6695 and MAX6696 have an additional set of temperature
         * limit registers. We can detect those chips by checking if
         * one of those registers exists.
         */
        else if (chip_id == 0x01 && (config1 & 0x10) == 0x00 && (status2 & 0x01) == 0x00 && emerg == emerg2 && convrate <= 0x07) {
            name = "Maxim MAX6696";
            dev->chip_id = MAX6559;
        }
        /*
         * The chip_id register of the MAX6680 and MAX6681 holds the
         * revision of the chip. The lowest bit of the config1 register
         * is unused and should return zero when read, so should the
         * second to last bit of config1 (software reset).
         */
        else if (chip_id == 0x01 && (config1 & 0x03) == 0x00 && convrate <= 0x07) {
            name = "Maxim MAX6680";
            dev->chip_id = LM99;
        }
        /*
         * The chip_id register of the MAX6646/6647/6649 holds the
         * revision of the chip. The lowest 6 bits of the config1
         * register are unused and should return zero when read.
         */
        else if (chip_id == 0x59 && (config1 & 0x3f) == 0x00 && convrate <= 0x07) {
            name = "Maxim MAX6646";
            dev->chip_id = MAX6559;
        }
    } else if (address == 0x4C && man_id == 0x5C) { /* Winbond/Nuvoton */
        if ((config1 & 0x2A) == 0x00 && (config2 & 0xF8) == 0x00) {
            if (chip_id == 0x01 /* W83L771W/G */ && convrate <= 0x09) {
                name = "Winbond/Nuvoton W83l771";
                dev->chip_id = MAX6559;
            } else if ((chip_id & 0xFE) == 0x10 /* W83L771AWG/ASG */ && convrate <= 0x08) {
                name = "Winbond/Nuvoton W83l771";
                dev->chip_id = LM99;
            }
        }
    } else if (address >= 0x48 && address <= 0x4F && man_id == 0xA1) { /*  NXP Semiconductor/Philips */
        if (chip_id == 0x00 && (config1 & 0x2A) == 0x00 && (config2 & 0xFE) == 0x00 && convrate <= 0x09) {
            name = "NXP Semiconductor/Philips SA56004";
            dev->chip_id = LM99;
        }
    } else if ((address == 0x4C || address == 0x4D) && man_id == 0x47) { /* GMT */
        if (chip_id == 0x01 /* G781 */ && (config1 & 0x3F) == 0x00 && convrate <= 0x08) {
            name = "GMT G781";
            dev->chip_id = LM99;
        }
    }
    
	if (!name) /* identification failed */
		return 0;
    
    dev->chip_name = STRDUP(name, sizeof(name));
    
    return 1;
}

int lm99_get_board_temp(nouveau_device *device)
{
    I2CByte temp;
    xf86I2CReadByte(device->nvclock_i2c_sensor, LM99_REG_LOCAL_TEMP, &temp);
    return temp;
}

int lm99_get_gpu_temp(nouveau_device *device)
{
    I2CByte temp;
    
    xf86I2CReadByte(device->nvclock_i2c_sensor, LM99_REG_REMOTE_TEMP, &temp);
    
    /* Cards with lm99 chips need an offset of 16C according to the datasheets. */
    if(device->nvclock_i2c_sensor->chip_id == LM99)
    {
        temp += 16;
    }
    
    /* The temperature needs to be corrected using an offset which is stored in the bios.
     /  If no bios has been parsed we fall back to a default value.
     */
    if(device->bios.data)
    {
        temp += device->sensor_constants.offset_constant; //nv_card->bios->sensor_cfg.temp_correction;
    }
    else
    {
        /* An extra offset of 10C seems to be needed on Geforce6800 cards to match nvidia-settings.
         /  Last but not least Geforce6600GT boards containing an LM99 sensor seem to need a +5C offset.
         */
        if(device->chipset == 0x43)
            temp += 5;
        else if(device->card_type == NV_40)
            temp += 10;
    }
    
    return temp;
}
