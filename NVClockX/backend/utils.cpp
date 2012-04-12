/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2007 Roderick Colenbrander
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

#include <stdio.h>

/* Convert the gpu architecture to a string using NVxx/Gxx naming */
int convert_gpu_architecture(short arch, char *buf)
{
	if(!buf)
		return 0;

	switch(arch)
	{
		case 0x46:
			sprintf(buf, "NV46/G72"); /* 7300 */
			break;
		case 0x47:
			sprintf(buf, "NV47/G70"); /* 7800 */
			break;
		case 0x49:
			sprintf(buf, "NV49/G71"); /* 7900 */
			break;
		case 0x4b:
			sprintf(buf, "NV4B/G73"); /* 7600 */
			break;
		case 0x4c: /* is this correct also a C51? */
		case 0x4e:
			sprintf(buf, "C51"); /* Geforce 6x00 nForce */
			break;
		// sprintf(buf, "C68"); /* Geforce 70xx nForce */ 
		case 0x50:
			sprintf(buf, "NV50/G80"); /* 8800 */
			break;
		case 0xa0:
			sprintf(buf, "GT200"); /* Geforce GTX260/280 */
			break;
		default:
			if(arch <= 0x44) /* The NV44/6200TC is the last card with only an NV name */
				sprintf(buf, "NV%X", arch);
			else /* Use Gxx for anything else */
				sprintf(buf, "G%X", arch);
	}
	return 1;
}

/* Convert a mask containing enabled/disabled pipelines for nv4x cards
/  to a binary string.
*/
void convert_unit_mask_to_binary(char mask, char hw_default, char *buf)
{
	int i, len;

	/* Count the number of pipelines on the card */
	for(i=0, len=0; i<8; i++)
		len += (hw_default & (1<<i)) ? 1 : 0;

	for(i=0; i<len; i++)
	{
		buf[i] = (mask & (1<<(len-i-1))) ? '1' : '0';
	}
	buf[len] = 0;
}
