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
#include <string.h>
#include "i2c.h"
#include "NouveauDefinitions.h"


/* various defines for register offsets and such are needed */

#define LM99_REG_LOCAL_TEMP 0x0
#define LM99_REG_REMOTE_TEMP 0x1
#define LM99_REG_MAN_ID 0xfe
#define NATSEM_MAN_ID 0x1
#define MAXIM_MAN_ID 0x4d
#define LM99_REG_CHIP_ID 0xff

/* This function should return the chip type .. */
int lm99_detect(I2CDevPtr dev)
{
	I2CByte man_id, chip_id;

	xf86I2CReadByte  (dev, LM99_REG_MAN_ID, &man_id); 
	xf86I2CReadByte  (dev, LM99_REG_CHIP_ID, &chip_id); 
  
	switch(man_id)
	{
		/* National Semiconductor LM99; needs offset? */
		case NATSEM_MAN_ID:
			dev->chip_id = LM99;
			dev->chip_name = (char*)STRDUP("National Semiconductor LM99", sizeof("National Semiconductor LM99"));
			break;
		/* Unknown vendor; this chip was used in a FX5700Go laptop and looks similar to the MAx6659 */
		case 0x47:
		/* Maxim; likely a 655x model */
		case MAXIM_MAN_ID:
			dev->chip_id = MAX6559;
			dev->chip_name = (char*)STRDUP("Maxim MAX6659", sizeof("Maxim MAX6659"));
			break;
		default:
			return 0;
	}
  
	return 1;
}

int lm99_get_board_temp(I2CDevPtr dev)
{
	I2CByte temp;
	xf86I2CReadByte(dev, LM99_REG_LOCAL_TEMP, &temp);
	return temp;
}

int lm99_get_gpu_temp(I2CDevPtr dev)
{
	I2CByte temp;
	
	xf86I2CReadByte(dev, LM99_REG_REMOTE_TEMP, &temp);

	/* Cards with lm99 chips need an offset of 16C according to the datasheets. */
	if(dev->chip_id == LM99)
	{
		temp += 16;
	}

	/* The temperature needs to be corrected using an offset which is stored in the bios.
	/  If no bios has been parsed we fall back to a default value.
	*/
	if(nouveau_card->bios.data)
	{
		temp += nouveau_card->sensor_constants.offset_constant; //nv_card->bios->sensor_cfg.temp_correction;
	}
	else
	{
		/* An extra offset of 10C seems to be needed on Geforce6800 cards to match nvidia-settings.
		/  Last but not least Geforce6600GT boards containing an LM99 sensor seem to need a +5C offset.
		*/
		if(nouveau_card->chipset == 0x43)
			temp += 5;
		else if(nouveau_card->card_type == NV_40)
			temp += 10;
	}
	
	return temp;
}
