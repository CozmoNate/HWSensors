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

/* PCI stuff */
#define PCI_VENDOR_ID 0x0 /* 16-bit */
# define PCI_VENDOR_ID_APPLE 0x106b
# define PCI_VENDOR_ID_HP 0x103c
# define PCI_VENDOR_ID_SANYO 0x144d /* Samsung laptops use the Sanyo vendor id */
# define PCI_VENDOR_ID_SONY 0x104d
# define PCI_VENDOR_ID_ZEPTO 0x1a46
#define PCI_DEVICE_ID 0x2 /* 16-bit */
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c /* 16-bit */
#define PCI_SUBSYSTEM_ID 0x2e /* 16-bit */
#define PCI_CAPABILITY_LIST 0x34
#define PCI_CAP_LIST_ID 0x0
#define PCI_CAP_LIST_NEXT 0x1
#define PCI_CAP_ID_AGP 0x2 /* AGP */
#define PCI_AGP_STATUS 0x4
# define PCI_AGP_STATUS_SBA 0x200  /* Sideband Addressing */
# define PCI_AGP_STATUS_64BIT 0x20
# define PCI_AGP_STATUS_FW 0x10  /* Fast Writes */
# define PCI_AGP_STATUS_RATE_8X_SUPPORT 0x8 /* If set AGP1x/AGP2x need to be interpreted as AGP4x/AGP8x */
# define PCI_AGP_STATUS_RATE_8X_SHIFT 0x2 /* Needs to be used when 8x support is enabled to translate 1x/ -> 4x/8x*/
# define PCI_AGP_STATUS_RATE_4X 0x4
# define PCI_AGP_STATUS_RATE_2X 0x2
# define PCI_AGP_STATUS_RATE_1X 0x1
# define PCI_AGP_STATUS_RATE_MASK 0x7 /* AGP4X | AGP2X | AGP1X */
#define PCI_AGP_COMMAND 0x8
# define PCI_AGP_COMMAND_SBA 0x200  /* Sideband Addressing */
# define PCI_AGP_COMMAND_AGP 0x100  /* Tells if AGP is enabled */
# define PCI_AGP_COMMAND_64BIT 0x20
# define PCI_AGP_COMMAND_FW 0x10  /* Fast Writes */
# define PCI_AGP_COMMAND_RATE_4X 0x4
# define PCI_AGP_COMMAND_RATE_2X 0x2
# define PCI_AGP_COMMAND_RATE_1X 0x1
# define PCI_AGP_COMMAND_RATE_MASK 0x7 /* AGP4X | AGP2X | AGP1X */
#define PCI_CAP_ID_EXP 0x10 /* PCI-Express */
#define PCIE_LINKCAP 0xc
#define PCIE_LINKCONTROL 0x10
#define PCIE_LINKSTATUS 0x12
# define PCIE_LINK_SPEED_MASK 0x3f0
# define PCIE_LINK_SPEED_SHIFT 4

/* PMC */
#define NV_PMC_BOOT_0 0x0
# define NV_PMC_BOOT_0_REVISION_MINOR 0xf
# define NV_PMC_BOOT_0_REVISION_MAJOR 0xf0 /* in general A or B, on pre-NV10 it was different */
# define NV_PMC_BOOT_0_REVISION_MASK 0xff

/* PDISPLAY */
#define NV_PDISPLAY_OFFSET 0x610000
#define NV_PDISPLAY_SIZE 0x10000
#define NV_PDISPLAY_SOR0_REGS_BRIGHTNESS 0xc084
# define NV_PDIPSLAY_SOR0_REGS_BRIGHTNESS_CONTROL_ENABLED 0x80000000

/* PRAMIN */
#define NV_PRAMIN_OFFSET 0x00700000
#define NV_PRAMIN_SIZE 0x00100000

/* PROM */
#define NV_PROM_OFFSET 0x300000
#define NV_PROM_SIZE 0xffff /* size in bytes */

/* NV4X registers
*
*  0xc040: used to enble/disable parts of the GPU?
*   bit1:0, enable/disable PLL 0x4000/0x4004; perhaps one bit is enable and the other a PLL layout switch?
*   bit3:2, ??
*   bit5:4, seems similar to bit1:0, perhaps for a VPLL? but in various cases it is equal to bit1:0
*   bit7:6, ??
*   bit9:8,  seems similar to bit1:0, perhaps for a VPLL? but in various cases it is equal to bit1:0
*   bit11:10, seems related to 0x4030/0x4034 (??)
*   bit12:13, ??
*   bit15:14, seems related to 0x4020/0x4024; perhaps it is for all MPLL ones? (what about bit11:10 then?)
*   bit17:16, seems related to 0x680508/0x680578 (primary VPLL)
*   bit29:28, seems to be off when dumping bios
*/

/* NV5x registers
*
*  The following register is around again but much more bits are used, so start a new description
*  0xc040: used to enable/disable parts of the GPU
*   bit21:20, used to enable/disable PLL 0x4008/0x400c (gpu clock)
*/
