//
//  i2c.c
//  HWSensors
//
//  Created by Kozlek on 31.07.12.
//
//

#include "NouveauDefinitions.h"
#include "nouveau.h"
#include "i2c.h"

void i2c_lock_unlock(int lock)
{
	unsigned char cr11;
    
    nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d4, 0x1f);
	nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5, lock ? 0x99 : 0x57);
    
	nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d4, 0x11);
	cr11 = nv_rd08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5);
	if(lock) cr11 |= 0x80;
	else cr11 &= ~0x80;
	nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5, cr11);
}


void i2c_get_bits(I2CBusPtr b, int *clock, int *data)
{
	unsigned char val;
	int DDCBase = (int)b->DriverPrivate.val;
    
	/* Get the result. */
	nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d4, DDCBase);
	val = nv_rd08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5);
    
	*clock = (val & DDC_SCL_READ_MASK) != 0;
	*data  = (val & DDC_SDA_READ_MASK) != 0;
}

void i2c_put_bits(I2CBusPtr b, int clock, int data)
{
	unsigned char val;
	int DDCBase = (int)b->DriverPrivate.val;
    
	nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d4, DDCBase + 1);
	val = nv_rd08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5) & 0xf0;
	if (clock)
		val |= DDC_SCL_WRITE_MASK;
	else
		val &= ~DDC_SCL_WRITE_MASK;
    
	if (data)
		val |= DDC_SDA_WRITE_MASK;
	else
		val &= ~DDC_SDA_WRITE_MASK;
    
	nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d4, DDCBase + 1);
	nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5, val | 0x1);
}

I2CBusPtr i2c_create_bus_ptr(char *name, int bus)
{
	I2CBusPtr I2CPtr;
    
	I2CPtr = xf86CreateI2CBusRec();
	if(!I2CPtr) return NULL;
    
	I2CPtr->BusName    = name;
	I2CPtr->scrnIndex  = nouveau_card->card_index; /* We need to use unique indices or else it can lead to a segfault in multicard situations */
	I2CPtr->I2CAddress = I2CAddress;
	I2CPtr->I2CPutBits = i2c_put_bits;
	I2CPtr->I2CGetBits = i2c_get_bits;
	I2CPtr->AcknTimeout = 5;
	I2CPtr->DriverPrivate.val = bus;
    
	if (!xf86I2CBusInit(I2CPtr))
	{
		return 0;
	}
	return I2CPtr;
}

void nv50_i2c_get_bits(I2CBusPtr bus, int *clock, int *data)
{
	const long offset = bus->DriverPrivate.val;
	unsigned char val;
    
	val = nv_rd08(nouveau_card, (0x0000E138 + offset)/4);
	*clock = !!(val & 1);
	*data = !!(val & 2);
}

void nv50_i2c_put_bits(I2CBusPtr bus, int clock, int data)
{
	const long offset = bus->DriverPrivate.val;
	nv_wr08(nouveau_card, (0x0000E138 + offset)/4, 4 | clock | data << 1);
}

I2CBusPtr nv50_i2c_create_bus_ptr(char *name, int bus)
{
	I2CBusPtr I2CPtr;
    
	I2CPtr = xf86CreateI2CBusRec();
	if(!I2CPtr) return NULL;
    
	I2CPtr->BusName    = name;
	I2CPtr->scrnIndex  = nouveau_card->card_index; /* We need to use unique indices or else it can lead to a segfault in multicard situations */
	I2CPtr->I2CAddress = I2CAddress;
	I2CPtr->I2CPutBits = nv50_i2c_put_bits;
	I2CPtr->I2CGetBits = nv50_i2c_get_bits;
	I2CPtr->AcknTimeout = 40;
	I2CPtr->DriverPrivate.val = bus;
    
	if (!xf86I2CBusInit(I2CPtr))
	{
		return 0;
	}
	return I2CPtr;
}

void i2c_probe_device (I2CBusPtr bus, I2CSlaveAddr addr, const char *format, ...)
{
	I2CDevPtr dev;
	char *s;
	va_list ap;
    
	if(xf86I2CProbeAddress(bus, addr))
	{
		dev = xf86CreateI2CDevRec();
		s = (char*)IOMalloc(8);
		va_start (ap, format);
		vsnprintf (s, 7, format, ap);
		va_end (ap);
		dev->DevName = s;
		dev->SlaveAddr = addr;
		dev->pI2CBus = bus;
        
		if (!xf86I2CDevInit(dev))
		{
			IOFree(dev->DevName, 8);
			xf86DestroyI2CDevRec(dev, TRUE);
		}
	}
}

void i2c_probe_all_devices (I2CBusPtr busses[], int nbus)
{
	I2CSlaveAddr addr;
	int bus;
	for (bus = 0; bus < nbus; bus++)
	{
		for (addr = 0x00; addr < 0x100; addr += 2)
		{
			i2c_probe_device (busses[bus], addr, "%1i:%02X", bus, addr);
		}
	}
}

I2CDevPtr i2c_probe_devices(I2CBusPtr busses[], int num_busses)
{
	int bus;
	I2CDevPtr dev;
    
	/* Unlock the extended CRTC registers to get i2c working */
	i2c_lock_unlock(0);
    
	/* On NV40 cards the i2c busses can be disabled */
	if(nouveau_card->card_type == NV_40)
	{
		nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d4, 0x49);
        unsigned char val = nv_rd08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5);
        nv_wr08(nouveau_card, NV_PRMCIO0_OFFSET + 0x3d5, val | 0x4);
        //nv_card->PCIO[0x3d5] |= 0x4; /* Unlock the i2c busses */
	}
	i2c_probe_all_devices(busses, num_busses);
    
    NouveauInfoLog("probing I2C busses");
    
	for(bus = 0; bus < num_busses; bus++)
	{
		for(dev = busses[bus]->FirstDev; dev; dev = dev->NextDev)
		{
			NouveauInfoLog("bus: %x device: %x", bus, dev->SlaveAddr);
            
			dev->arch = nouveau_card->chipset;
            
			switch(dev->SlaveAddr)
			{
                    /* LM99 */
				case 0x98:
					if(lm99_detect(dev))
						return dev;
					break;
				case 0x5a:
					if(w83l785r_detect(dev))
						return dev;
					if(w83781d_detect(dev))
						return dev;
				case 0x5c:
					if(f75375_detect(dev))
						return dev;
					if(adt7473_detect(dev))
						return dev;
				case 0x6e: /* DDC/CI ? */
                    /* The addresses below oftenly appear on most cards but what are these? */
				case 0x70:
				case 0xa0:
				case 0xa2:
				case 0xa4:
				case 0xa6:
				case 0xa8:
				case 0xaa:
				case 0xac:
				case 0xae:
					break;
				default:
                    /* Unknown device */
					break;
			}
		}
	}
    
	i2c_lock_unlock(1);
    
	return NULL;
}
