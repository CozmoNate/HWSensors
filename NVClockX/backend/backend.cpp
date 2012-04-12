/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 * 
 * Copyright(C) 2001-2007 Roderick Colenbrander
 *
 * site: http://nvclock.sourceforge.net
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#ifdef HAVE_NVCONTROL
    #include "nvcontrol.h"
#endif
#include "backend.h"

/* Read a byte from the pci bus */
unsigned char nv_read_pbus8(int offset)
{
	int shift = (offset % 4)*8;
	return (nv_card->PBUS[offset/4] >> shift) & 0xff;
}

/* Read an unsigned short from the pci bus */
unsigned short nv_read_pbus16(int offset)
{
	int shift = (offset / 2)*16;
	return (nv_card->PBUS[offset/4] >> shift) & 0xffff;
}

/* Read an unsigned int from the pci bus */
unsigned int nv_read_pbus(int offset)
{
	return nv_card->PBUS[offset/4];
}

/* Read an unsigned int from the PMC registers */
unsigned int nv_read_pmc(int offset)
{
	return nv_card->PMC[offset/4];
}

/* This function is actually a basic version of set_card.
/  It mainly copies the entries of the card list and maps 
/  the video registers. We need this function because we need
/  access to the videocard from the config file creation code.
/  At that stage we can't use the normal set_card because that
/  function also sets function pointers and uses bios/config
/  file info which we don't have yet.
*/
int set_card_info(int number)
{
	nv_card = &nvclock.card[number];

	if(!nv_card->mem_mapped)
		if(!map_mem(nv_card->dev_name))
			return 0; /* map_mem already took care of the error */
    
	return 1;
}

/* Set the card object to the requested card */
int set_card(int number)
{
	int have_coolbits, irq;

	if(!set_card_info(number))
		return 0;

/* nvcontrol detection  */
#ifdef HAVE_NVCONTROL
	/* We need an X display to check if NV-CONTROL support exists */
	if(nvclock.dpy)
	{
		/* Check if we have NV-CONTROL support */
		if(init_nvcontrol(nvclock.dpy))
		{
			int tmp;
			have_coolbits = NVGetAttribute(nvclock.dpy, 0, 0, NV_GPU_OVERCLOCKING_STATE, &tmp);

			/* Also retrieve the irq which is used to sync nvclock and NV-CONTROL */
			NVGetAttribute(nvclock.dpy, 0, 0, NV_IRQ, &irq);
			if(have_coolbits && (nv_card->irq == irq))
			{
				nv_card->caps |= COOLBITS_OVERCLOCKING;
				
				/* By default use Coolbits on NV3X / NV4X cards */
				if(!nv_card->state && nv_card->arch & (NV3X|NV4X))
					nv_card->state = STATE_3D;
			}
		}
	}
#endif /* HAVE_NVCONTROL */
	
	info_init();

	if(nv_card->arch & NV3X)
		nv30_init();
	else if(nv_card->arch & NV4X)
		nv40_init();
	else if(nv_card->arch & NV5X)
		nv50_init();
	else
		nv_init();

	return 1;
}

void unset_card()
{
	unmap_mem();
}
