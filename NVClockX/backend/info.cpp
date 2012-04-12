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
#include <stdlib.h>
#include <string.h>
#include "backend.h"
#include "nvclock.h"

/* This list isn't used much for the speed ranges anymore */
/* Mainly mobile gpu speeds are missing */
static const struct pci_ids ids[] =
{
	{ 0x0020, "RIVA [NV4 TNT]", DESKTOP },
	{ 0x0028, "NV5 [RIVA TNT2/TNT2 Pro]", DESKTOP },
	{ 0x002a, "NV5 [Riva TNT2]", DESKTOP },
	{ 0x002b, "NV5 [Riva TNT2]", DESKTOP },
	{ 0x00a0, "NV5 [Aladdin TNT2]", DESKTOP },
	{ 0x002c, "NV6 [Vanta/Vanta LT]", DESKTOP },
	{ 0x002d, "NV5M64 [RIVA TNT2 Model 64/Model 64 Pro]", DESKTOP },
	{ 0x002e, "NV6 [Vanta]", DESKTOP },
	{ 0x002f, "NV6 [Vanta]", DESKTOP },
	{ 0x0029, "NV5 [RIVA TNT2 Ultra]", DESKTOP },
	{ 0x0100, "NV10 [GeForce 256 SDR]", DESKTOP },
	{ 0x0101, "NV10DDR [GeForce 256 DDR]", DESKTOP },
	{ 0x0103, "NV10GL [Quadro]", DESKTOP },
	{ 0x0110, "NV11 [GeForce2 MX/MX 400]", DESKTOP },
	{ 0x0111, "NV11DDR [GeForce2 MX200]", DESKTOP },
	{ 0x0112, "NV11 [GeForce2 Go]", MOBILE },
	{ 0x0113, "NV11GL [Quadro2 MXR/EX/Go]", MOBILE },
	{ 0x01a0, "nVidia Geforce 2 MX integrated [?]", NFORCE },
	{ 0x0150, "NV15 [GeForce2 GTS/Pro]", DESKTOP },
	{ 0x0151, "NV15DDR [GeForce2 Ti]", DESKTOP },
	{ 0x0152, "NV15BR [GeForce2 Ultra, Bladerunner]", DESKTOP },
	{ 0x0153, "NV15GL [Quadro2 Pro]", DESKTOP },
	{ 0x0170, "NV17 [GeForce4 MX 460]", DESKTOP },
	{ 0x0171, "NV17 [GeForce4 MX 440]", DESKTOP },
	{ 0x0172, "NV17 [GeForce4 MX 420]", DESKTOP },
	{ 0x0173, "NV17 [GeForce4 MX 440-SE]", DESKTOP },
	{ 0x0174, "NV17 [GeForce4 440 Go]", MOBILE },
	{ 0x0175, "NV17 [GeForce4 420 Go]", MOBILE },
	{ 0x0176, "NV17 [GeForce4 420 Go 32M]", MOBILE },
	{ 0x0177, "NV17 [GeForce4 460 Go]", MOBILE },
	{ 0x0178, "NV17GL [Quadro4 550 XGL]", DESKTOP },
	{ 0x0179, "NV17 [GeForce4 440 Go 64M]", MOBILE },
	{ 0x017a, "NV17GL [Quadro NVS]", DESKTOP },
	{ 0x017b, "NV17GL [Quadro4 550 XGL]", DESKTOP },
	{ 0x017c, "NV17GL [Quadro4 500 GoGL]", MOBILE },
	{ 0x017d, "NV17 [GeForce4 410 Go 16M]", MOBILE },
	{ 0x0180, "nVidia Geforce 4 MX440 8X [?]", DESKTOP },
	{ 0x0181, "NV18 [GeForce4 MX 440 AGP 8x]", DESKTOP },
	{ 0x0182, "NV18 [GeForce4 MX 440SE AGP 8x]", DESKTOP },
	{ 0x0185, "NV18 [GeForce4 MX 4000]", DESKTOP },
	{ 0x0183, "NV18 [GeForce4 MX 420 AGP 8x]", DESKTOP },
	{ 0x0186, "NV18M [GeForce4 448 Go]", MOBILE },
	{ 0x0187, "NV18M [GeForce4 488 Go]", MOBILE },
	{ 0x0188, "NV18GL [Quadro4 580 XGL]", DESKTOP },
	{ 0x018a, "NV18GL [Quadro NVS 280 SD]", DESKTOP },
	{ 0x018b, "NV18GL [Quadro4 380 XGL]", DESKTOP },
	{ 0x018c, "NV18GL [Quadro NVS 50 PCI]", DESKTOP },
	{ 0x018d, "NV18M [GeForce4 448 Go]", MOBILE },
	{ 0x01f0, "NV18 [GeForce4 MX - nForce GPU]", NFORCE },
	{ 0x0200, "NV20 [GeForce3]", DESKTOP },
	{ 0x0201, "NV20 [GeForce3 Ti 200]", DESKTOP },
	{ 0x0202, "NV20 [GeForce3 Ti 500]", DESKTOP },
	{ 0x0203, "NV20DCC [Quadro DCC]", DESKTOP },
	{ 0x0250, "NV25 [GeForce4 Ti 4600]", DESKTOP },
	{ 0x0251, "NV25 [GeForce4 Ti 4400]", DESKTOP },
	{ 0x0253, "NV25 [GeForce4 Ti 4200]", DESKTOP },
	{ 0x0258, "NV25GL [Quadro4 900 XGL]", DESKTOP },
	{ 0x0259, "NV25GL [Quadro4 750 XGL]", DESKTOP },
	{ 0x025a, "nVidia Quadro 4 600 XGL [?]", DESKTOP },
	{ 0x025b, "NV25GL [Quadro4 700 XGL]", DESKTOP },
	{ 0x0280, "NV28 [GeForce4 Ti 4800]", DESKTOP },
	{ 0x0281, "NV28 [GeForce4 Ti 4200 AGP 8x]", DESKTOP },
	{ 0x0282, "NV28 [GeForce4 Ti 4800 SE]", DESKTOP },
	{ 0x0286, "NV28 [GeForce4 Ti 4200 Go AGP 8x]", MOBILE },
	{ 0x0288, "NV28GL [Quadro4 980 XGL]", DESKTOP },
	{ 0x0289, "NV28GL [Quadro4 780 XGL]", DESKTOP },
	{ 0x028c, "NV28GLM [Quadro4 Go700]", MOBILE },
	{ 0x0300, "NV30 [GeForce FX]", DESKTOP },
	{ 0x0301, "NV30 [GeForce FX 5800 Ultra]", DESKTOP },
	{ 0x0302, "NV30 [GeForce FX 5800]", DESKTOP },
	{ 0x0308, "NV30GL [Quadro FX 2000]", DESKTOP },
	{ 0x0309, "NV30GL [Quadro FX 1000]", DESKTOP },
	{ 0x0311, "NV31 [GeForce FX 5600 Ultra]", DESKTOP },
	{ 0x0312, "NV31 [GeForce FX 5600]", DESKTOP },
	{ 0x0314, "NV31 [GeForce FX 5600XT]", DESKTOP },
	{ 0x0316, "NV31M [?]", MOBILE },
	{ 0x0317, "NV31M Pro [?]", MOBILE },
	{ 0x0318, "nVidia NV31GL [?]", DESKTOP },
	{ 0x0319, "nVidia NV31GL [?]", DESKTOP },
	{ 0x031a, "NV31M [GeForce FX Go5600]", MOBILE },
	{ 0x031b, "NV31M [GeForce FX Go5650]", MOBILE },
	{ 0x031c, "NV31 [Quadro FX Go700]", MOBILE },
	{ 0x031d, "NV31GLM [?]", MOBILE },
	{ 0x031e, "NV31GLM Pro [?]", MOBILE },
	{ 0x031f, "NV31GLM Pro [?]", MOBILE },
	{ 0x0321, "NV34 [GeForce FX 5200 Ultra]", DESKTOP },
	{ 0x0322, "NV34 [GeForce FX 5200]", DESKTOP },
	{ 0x0323, "NV34 [GeForce FX 5200LE]", DESKTOP },
	{ 0x0324, "NV34M [GeForce FX Go5200 64M]", MOBILE },
	{ 0x0325, "NV34M [GeForce FX Go5250]", MOBILE },
	{ 0x0326, "NV34 [GeForce FX 5500]", DESKTOP },
	{ 0x0328, "NV34M [GeForce FX Go5200 32M/64M]", MOBILE },
	{ 0x0329, "NV34M [GeForce FX Go5200]", MOBILE },
	{ 0x032a, "NV34GL [Quadro NVS 280 PCI]", DESKTOP },
	{ 0x032b, "NV34GL [Quadro FX 500/600 PCI]", DESKTOP },
	{ 0x032c, "NV34GLM [GeForce FX Go 5300]", MOBILE },
	{ 0x032d, "NV34 [GeForce FX Go5100]", MOBILE },
	{ 0x032f, "NV34GL [?]", DESKTOP },
	{ 0x0330, "NV35 [GeForce FX 5900 Ultra]", DESKTOP },
	{ 0x0331, "NV35 [GeForce FX 5900]", DESKTOP },
	{ 0x0332, "NV35 [GeForce FX 5900XT]", DESKTOP },
	{ 0x0333, "NV38 [GeForce FX 5950 Ultra]", DESKTOP },
	{ 0x0334, "NV35 [GeForce FX 5900ZT]", DESKTOP },
	{ 0x0338, "NV35GL [Quadro FX 3000]", DESKTOP },
	{ 0x033f, "NV35GL [Quadro FX 700]", DESKTOP },
	{ 0x0341, "NV36.1 [GeForce FX 5700 Ultra]", DESKTOP },
	{ 0x0342, "NV36.2 [GeForce FX 5700]", DESKTOP },
	{ 0x0343, "NV36 [GeForce FX 5700LE]", DESKTOP },
	{ 0x0344, "NV36.4 [GeForce FX 5700VE]", DESKTOP },
	{ 0x0345, "NV36.5 [?]", DESKTOP },
	{ 0x0347, "NV36 [GeForce FX Go5700]", MOBILE },
	{ 0x0348, "NV36 [GeForce FX Go5700]", MOBILE },
	{ 0x0349, "NV36M Pro [?]", MOBILE },
	{ 0x034b, "NV36MAP [?]", MOBILE },
	{ 0x034c, "NV36 [Quadro FX Go1000]", MOBILE },
	{ 0x034e, "NV36GL [Quadro FX 1100]", DESKTOP },
	{ 0x034f, "NV36GL [?]", DESKTOP },
	{ 0x02a0, "NV2A [XGPU] Xbox Graphics Processing Unit (Integrated).", NFORCE },
	{ 0x0040, "NV40 [GeForce 6800 Ultra]", DESKTOP },
	{ 0x0041, "NV40 [GeForce 6800]", DESKTOP },
	{ 0x0042, "NV40.2 [GeForce 6800 LE]", DESKTOP },
	{ 0x0043, "NV40.3 [GeForce 6800 XE]", DESKTOP },
	{ 0x0044, "NV40 [GeForce 6800 XT]", DESKTOP },
	{ 0x0045, "NV40 [GeForce 6800 GT]", DESKTOP },
	{ 0x0046, "NV45 [GeForce 6800 GT]", DESKTOP },
	{ 0x0047, "NV40 [GeForce 6800 GS]", DESKTOP },
	{ 0x0048, "NV40 [GeForce 6800 XT]", DESKTOP },
	{ 0x0049, "NV40GL [?]", DESKTOP },
	{ 0x004d, "NV40GL [Quadro FX 4000]", DESKTOP },
	{ 0x004e, "NV40GL [Quadro FX 4000]", DESKTOP },
	{ 0x00c0, "NV41 [GeForce 6800 GS]", DESKTOP },
	{ 0x00c1, "NV41.1 [GeForce 6800]", DESKTOP },
	{ 0x00c2, "NV41.2 [GeForce 6800 LE]", DESKTOP },
	{ 0x00c3, "NV42 [GeForce 6800 XT]", DESKTOP },
	{ 0x00c8, "NV41.8 [GeForce Go 6800]", MOBILE },
	{ 0x00c9, "NV41.9 [GeForce Go 6800 Ultra]", MOBILE },
	{ 0x00cc, "NV41 [Quadro FX Go1400]", MOBILE },
	{ 0x00cd, "NV41 [Quadro FX 3450/4000 SDI]", DESKTOP },
	{ 0x00ce, "NV41GL [Quadro FX 1400]", DESKTOP },
	{ 0x00f0, "NV40 [GeForce 6800 Ultra]", DESKTOP },
	{ 0x00f1, "NV43 [GeForce 6600 GT]", DESKTOP },
	{ 0x00f2, "NV43 [GeForce 6600]", DESKTOP },
	{ 0x00f3, "NV43 [GeForce 6200]", DESKTOP },
	{ 0x00f4, "NV43 [GeForce 6600 LE]", DESKTOP },
	{ 0x00f5, "G70 [GeForce 7800 GS]", DESKTOP },
	{ 0x00f6, "NV43 [GeForce 6800 GS]", DESKTOP },
	{ 0x00f8, "NV45GL [Quadro FX 3400/4400]", DESKTOP },
	{ 0x00f9, "NV45 [GeForce 6800 GTO]", DESKTOP },
	{ 0x00fa, "NV36 [GeForce PCX 5750]", DESKTOP },
	{ 0x00fb, "NV35 [GeForce PCX 5900]", DESKTOP },
	{ 0x00fc, "NV37GL [Quadro FX 330/GeForce PCX 5300]", DESKTOP },
	{ 0x00fd, "NV37GL [Quadro PCI-E Series]", DESKTOP },
	{ 0x00fe, "NV38GL [Quadro FX 1300]", DESKTOP },
	{ 0x00ff, "NV18 [GeForce PCX 4300]", DESKTOP },
	{ 0x0140, "NV43 [GeForce 6600 GT]", DESKTOP },
	{ 0x0141, "NV43 [GeForce 6600]", DESKTOP },
	{ 0x0142, "NV43 [GeForce 6600 LE]", DESKTOP },
	{ 0x0143, "NV43 [GeForce 6600 VE]", DESKTOP },
	{ 0x0144, "NV43 [GeForce Go 6600]", MOBILE },
	{ 0x0145, "NV43 [GeForce 6610 XL]", DESKTOP },
	{ 0x0146, "NV43 [Geforce Go 6600TE/6200TE]", MOBILE },
	{ 0x0147, "NV43 [GeForce 6700 XL]", DESKTOP },
	{ 0x0148, "NV43 [GeForce Go 6600]", MOBILE },
	{ 0x0149, "NV43 [GeForce Go 6600 GT]", MOBILE },
	{ 0x014a, "NV43 [Quadro NVS 440]", DESKTOP },
	{ 0x014b, "NV43 [?]", DESKTOP },
	{ 0x014c, "NV43 [Quadro FX 540 MXM]", DESKTOP },
	{ 0x014d, "NV43GL [Quadro FX 550]", DESKTOP },
	{ 0x014e, "NV43GL [Quadro FX 540]", DESKTOP },
	{ 0x014f, "NV43 [GeForce 6200]", DESKTOP },
	{ 0x0160, "NV44 [GeForce 6500]", DESKTOP },
	{ 0x0161, "NV44 [GeForce 6200 TurboCache(TM)]", DESKTOP },
	{ 0x0162, "NV44 [GeForce 6200SE TurboCache (TM)]", DESKTOP },
	{ 0x0163, "NV44 [GeForce 6200 LE]", DESKTOP },
	{ 0x0164, "NV44 [GeForce Go 6200]", MOBILE },
	{ 0x0165, "NV44 [Quadro NVS 285]", DESKTOP },
	{ 0x0166, "NV43 [GeForce Go 6400]", MOBILE },
	{ 0x0167, "NV43 [GeForce Go 6200/6400]", MOBILE },
	{ 0x0168, "NV43 [GeForce Go 6200/6400]", MOBILE },
	{ 0x0169, "NV44 [GeForce 6250]", DESKTOP },
	{ 0x016a, "NV44 [GeForce 7100 GS]", DESKTOP },
	{ 0x016b, "NV44GLM [?]", DESKTOP },
	{ 0x016c, "NV44GLM [?]", DESKTOP },
	{ 0x016d, "NV44GLM [?]", DESKTOP },
	{ 0x016e, "NV44GLM [?]", DESKTOP },
	{ 0x0210, "NV48 [?]", DESKTOP },
	{ 0x0211, "NV48 [GeForce 6800]", DESKTOP },
	{ 0x0212, "NV48 [GeForce 6800 LE]", DESKTOP },
	{ 0x0215, "NV48 [GeForce 6800 GT]", DESKTOP },
	{ 0x0218, "NV48 [GeForce 6800 XT]", DESKTOP },
	{ 0x0220, "NV44 [?]", DESKTOP },
	{ 0x0221, "NV44A [GeForce 6200]", DESKTOP },
	{ 0x0222, "NV44 [GeForce 6200 A-LE]", DESKTOP },
	{ 0x0228, "NV44M [?]", MOBILE },
	{ 0x0240, "C51PV [GeForce 6150]", NFORCE },
	{ 0x0241, "C51 [GeForce 6150 LE]", NFORCE },
	{ 0x0242, "C51G [GeForce 6100]", NFORCE },
	{ 0x0244, "C51 [Geforce Go 6150]", NFORCE },
	{ 0x0245, "C51 [Quadro NVS 210S/GeForce 6150LE]", NFORCE },
	{ 0x0247, "C51 [GeForce Go 6100]", NFORCE },
	{ 0x02dd, "NV4x [?]", DESKTOP },
	{ 0x02de, "NV4x [?]", DESKTOP },
	{ 0x0090, "G70 [GeForce 7800 GTX]", DESKTOP },
	{ 0x0091, "G70 [GeForce 7800 GTX]", DESKTOP },
	{ 0x0092, "G70 [GeForce 7800 GT]", DESKTOP },
	{ 0x0093, "G70 [GeForce 7800 GS]", DESKTOP },
	{ 0x0094, "G70 [?]", DESKTOP },
	{ 0x0098, "G70 [GeForce Go 7800]", DESKTOP },
	{ 0x0099, "G70 [GeForce Go 7800 GTX]", MOBILE },
	{ 0x009c, "G70 [?]", DESKTOP },
	{ 0x009d, "G70GL [Quadro FX 4500]", DESKTOP },
	{ 0x009e, "G70GL [?]", DESKTOP },
	{ 0x0290, "G71 [GeForce 7900 GTX]", DESKTOP },
	{ 0x0291, "G71 [GeForce 7900 GT/GTO]", DESKTOP },
	{ 0x0292, "G71 [GeForce 7900 GS]", DESKTOP },
	{ 0x0293, "G71 [GeForce 7900 GX2]", DESKTOP },
	{ 0x0294, "G71 [GeForce 7950 GX2]", DESKTOP },
	{ 0x0295, "G71 [GeForce 7950 GT]", DESKTOP },
	{ 0x0297, "G71 [GeForce Go 7950 GTX]", MOBILE },
	{ 0x0298, "G71 [GeForce Go 7900 GS]", MOBILE },
	{ 0x0299, "G71 [GeForce Go 7900 GTX]", MOBILE },
	{ 0x029a, "G71 [Quadro FX 2500M]", MOBILE },
	{ 0x029b, "G71 [Quadro FX 1500M]", MOBILE },
	{ 0x029c, "G71 [Quadro FX 5500]", DESKTOP },
	{ 0x029d, "G71GL [Quadro FX 3500]", DESKTOP },
	{ 0x029e, "G71 [Quadro FX 1500]", DESKTOP },
	{ 0x029f, "G70 [Quadro FX 4500 X2]", DESKTOP },
	{ 0x0390, "G73 [GeForce 7650 GS]", DESKTOP },
	{ 0x0391, "G73 [GeForce 7600 GT]", DESKTOP },
	{ 0x0392, "G73 [GeForce 7600 GS]", DESKTOP },
	{ 0x0393, "G73 [GeForce 7300 GT]", DESKTOP },
	{ 0x0394, "G73 [GeForce 7600 LE]", DESKTOP },
	{ 0x0395, "G73 [GeForce 7300 GT]", DESKTOP },
	{ 0x0397, "G73 [GeForce Go 7700]", MOBILE },
	{ 0x0398, "G73 [GeForce Go 7600]", MOBILE },
	{ 0x0399, "G73 [GeForce Go 7600 GT]", MOBILE },
	{ 0x039a, "G73M [Quadro NVS 300M]", MOBILE },
	{ 0x039b, "G73 [GeForce Go 7900 SE]", MOBILE },
	{ 0x039c, "G73 [Quadro FX 550M]", MOBILE },
	{ 0x039e, "G73GL [Quadro FX 560]", DESKTOP },
	{ 0x02e0, "G73 [GeForce 7600 GT]", DESKTOP },
	{ 0x02e1, "G73 [GeForce 7600 GS]", DESKTOP },
	{ 0x02e2, "G73 [GeForce 7300 GT]", DESKTOP },
	{ 0x02e4, "G71 [GeForce 7950 GT]", DESKTOP },
	{ 0x01d1, "G72 [GeForce 7300 LE]", DESKTOP },
	{ 0x01d3, "G72 [GeForce 7300 SE/7200 GS]", DESKTOP },
	{ 0x01d7, "G72M [Quadro NVS 110M/GeForce Go 7300]", MOBILE },
	{ 0x01d8, "G72M [GeForce Go 7400]", MOBILE },
	{ 0x01d9, "G72M [GeForce Go 7450]", MOBILE },
	{ 0x01da, "G72M [Quadro NVS 110M]", MOBILE },
	{ 0x01db, "G72M [Quadro NVS 120M]", MOBILE },
	{ 0x01dc, "G72GL [Quadro FX 350M]", MOBILE },
	{ 0x01dd, "G72 [GeForce 7500 LE]", MOBILE },
	{ 0x01de, "G72GL [Quadro FX 350]", DESKTOP },
	{ 0x01df, "G71 [GeForce 7300 GS]", DESKTOP },
	{ 0x0190, "G80 [GeForce 8800]", DESKTOP },
	{ 0x0191, "G80 [GeForce 8800 GTX]", DESKTOP },
	{ 0x0192, "G80 [GeForce 8800]", DESKTOP },
	{ 0x0193, "G80 [GeForce 8800 GTS]", DESKTOP },
	{ 0x0194, "G80 [GeForce 8800 Ultra]", DESKTOP },
	{ 0x0197, "G80 [Tesla C870]", DESKTOP },
	{ 0x019a, "G80 [G80-875]", DESKTOP },
	{ 0x019d, "G80 [Quadro FX 5600]", DESKTOP },
	{ 0x019e, "G80 [Quadro FX 4600]", DESKTOP },
	{ 0x0400, "G84 [GeForce 8600 GTS]", DESKTOP },
	{ 0x0401, "G84 [GeForce 8600GT]", DESKTOP },
	{ 0x0402, "G84 [GeForce 8600 GT]", DESKTOP },
	{ 0x0403, "G84 [GeForce 8600 GS]", DESKTOP },
	{ 0x0404, "G84 [GeForce 8400 GS]", DESKTOP },
	{ 0x0405, "G84 [GeForce 9500M GS]", MOBILE },
	{ 0x0406, "G84 [GeForce 8300 GS]", MOBILE },
	{ 0x0407, "G84 [GeForce 8600M GT]", MOBILE },
	{ 0x0408, "G84 [GeForce 9650M GS]", MOBILE },
	{ 0x0409, "G84 [GeForce 8700M GT]", MOBILE },
	{ 0x040a, "G84 [Quadro FX 370]", MOBILE },
	{ 0x040b, "G84M [Quadro NVS 320M]", MOBILE },
	{ 0x040c, "G84M [Quadro FX 570M]", MOBILE },
	{ 0x040d, "G84 [Quadro FX 1600M]", MOBILE },
	{ 0x040e, "G84 [Quadro FX 570]", DESKTOP },
	{ 0x040f, "G84 [Quadro FX 1700]", DESKTOP },
	{ 0x0420, "G86 [GeForce 8400 SE]", DESKTOP },
	{ 0x0421, "G86 [GeForce 8500 GT]", DESKTOP },
	{ 0x0422, "G86 [GeForce 8400 GS]", DESKTOP },
	{ 0x0423, "G86 [GeForce 8300 GS]", DESKTOP },
	{ 0x0424, "G86 [GeForce 8400 GS]", DESKTOP },
	{ 0x0425, "G86 [GeForce 8600M GS]", MOBILE },
	{ 0x0426, "G86 [GeForce 8400M GT]", MOBILE },
	{ 0x0427, "G86 [GeForce 8400M GS]", MOBILE },
	{ 0x0428, "G86 [GeForce 8400M G]", MOBILE },
	{ 0x0429, "G84M [Quadro NVS 140M]", MOBILE },
	{ 0x042a, "G86M [Quadro NVS 130M]", MOBILE },
	{ 0x042b, "G86M [Quadro NVS 135M]", MOBILE },
	{ 0x042d, "G86M [Quadro FX 360M]", MOBILE },
	{ 0x042e, "G86 [GeForce 9300M G]", MOBILE },
	{ 0x042f, "G86 [Quadro NVS 290]", DESKTOP },
	{ 0x03d0, "C61 [GeForce 6150SE nForce 430]", NFORCE},
	{ 0x03d1, "C61 [GeForce 6100 nForce 405]", NFORCE},
	{ 0x03d2, "C61 [GeForce 6100 nForce 400]", NFORCE},
	{ 0x03d5, "C61 [GeForce 6100 nForce 420]", NFORCE},
	{ 0x053a, "C68 [GeForce 7050 PV / nForce 630a]", NFORCE},
	{ 0x053b, "C68 [GeForce 7050 PV / nForce 630a]", NFORCE},
	{ 0x053e, "C68 [GeForce 7025 / nForce 630a]", NFORCE},
	{ 0x05e0, "GT200b [GeForce GTX 295]", DESKTOP },
	{ 0x05e1, "GT200 [GeForce GTX 280]", DESKTOP },
	{ 0x05e2, "GT200 [GeForce GTX 260]", DESKTOP },
	{ 0x05e7, "GT200 [Tesla C1060]", DESKTOP },
	{ 0x05ed, "GT200GL [Quadro Plex 2200 D2]", DESKTOP },
	{ 0x05f8, "GT200GL [Quadro Plex 2200 S4]", DESKTOP },
	{ 0x05f9, "GT200GL [Quadro CX]", DESKTOP },
	{ 0x05fd, "GT200GL [Quadro FX 5800]", DESKTOP },
	{ 0x05fe, "GT200GL [Quadro FX 4800]", DESKTOP },
	{ 0x0600, "G92 [GeForce 8800 GTS 512]", DESKTOP },
	{ 0x0602, "G92 [GeForce 8800 GT]", DESKTOP },
	{ 0x0604, "G92 [GeForce 9800 GX2]", DESKTOP },
	{ 0x0606, "G92 [GeForce 8800 GS]", DESKTOP },
	{ 0x060d, "G92 [GeForce 8800 GS]", DESKTOP },
	{ 0x0609, "G92 [GeForce 8800M GTS]", MOBILE },
	{ 0x060c, "G92 [GeForce 8800M GTX]", MOBILE },
	{ 0x0610, "G92 [GeForce 9600 GSO]", DESKTOP },
	{ 0x0611, "G92 [GeForce 8800 GT]", DESKTOP },
	{ 0x0612, "G92 [GeForce 9800 GTX]", DESKTOP },
	{ 0x0614, "G92 [GeForce 9800 GT]", DESKTOP },
	{ 0x061a, "G92 [Quadro FX 3700]", DESKTOP },
	{ 0x061c, "G92M [Quadro FX 3600M]", MOBILE },
	{ 0x0622, "G94 [GeForce 9600 GT]", DESKTOP },
	{ 0x0623, "G94 [GeForce 9600 GS]", DESKTOP },
	{ 0x0640, "G96 [GeForce 9500 GT]", DESKTOP },
	{ 0x0643, "G96 [GeForce 9500 GT]", DESKTOP },
	{ 0x0647, "G96 [GeForce 9600M GT]", MOBILE },
	{ 0x0648, "G96 [GeForce 9600M GS]", MOBILE },
	{ 0x0649, "G96 [GeForce 9600M GT]", MOBILE },
	{ 0x064b, "G96 [GeForce 9500M G]", MOBILE },
	{ 0x06e0, "G98 [GeForce 9300 GE]", DESKTOP },
	{ 0x06e1, "G98 [GeForce 9300 GS]", DESKTOP },
	{ 0x06e2, "G98 [GeForce 8400]", DESKTOP },
	{ 0x06e3, "G98 [GeForce 8300 GS]", DESKTOP },
	{ 0x06e4, "G98 [GeForce 8400 GS]", DESKTOP },
	{ 0x06e5, "G98 [GeForce 9300M GS]", MOBILE },
	{ 0x06e6, "G98 [GeForce G100]", DESKTOP },
	{ 0x06e7, "G98 [GeForce 9300 SE]", DESKTOP },
	{ 0x06e8, "G98 [GeForce 9200M GS]", MOBILE },
	{ 0x06e9, "G98 [GeForce 9300M GS]", MOBILE },
	{ 0x06ea, "G86M [Quadro NVS 150M]", MOBILE },
	{ 0x06eb, "G98M [Quadro NVS 160M]", MOBILE },
	{ 0x06ec, "G98M [GeForce G 105M]", MOBILE },
	{ 0x06ed, "G98 [?]", DESKTOP },
	{ 0x06ee, "G98 [?]", DESKTOP },
	{ 0x06ef, "G98M [GeForce G 103M]", DESKTOP },
	{ 0x06f0, "G98 [?]", DESKTOP },
	{ 0x06f1, "G98 [GeForce G105M]", DESKTOP },
	{ 0x06f2, "G98 [?]", DESKTOP },
	{ 0x06f3, "G98 [?]", DESKTOP },
	{ 0x06f4, "G98 [?]", DESKTOP },
	{ 0x06f5, "G98 [?]", DESKTOP },
	{ 0x06f6, "G98 [?]", DESKTOP },
	{ 0x06f7, "G98 [?]", DESKTOP },
	{ 0x06f8, "G98 [Quadro NVS 420]", DESKTOP },
	{ 0x06f9, "G98 [Quadro FX 370 LP]", DESKTOP },
	{ 0x06fa, "G98 [Quadro NVS 450]", DESKTOP },
	{ 0x06fb, "G98 [Quadro FX 370M]", MOBILE },
	{ 0x06fc, "G98 [?]", DESKTOP },
	{ 0x06fd, "G98 [Quadro NVS 295]", DESKTOP },
	{ 0x06fe, "G98 [?]", DESKTOP },
	{ 0x06ff, "G98 [HICx16 + Graphics]", DESKTOP },
	{ 0x0860, "C79 [GeForce 9300]", DESKTOP },
	{ 0x0861, "C79 [GeForce 9400]", DESKTOP },
	{ 0x0863, "C79 [GeForce 9400M]", MOBILE },
	{ 0x0864, "C79 [GeForce 9300]", DESKTOP },
	{ 0x0865, "C79 [GeForce 9300]", DESKTOP },


	/* New product-ids */
	{ 0x0095, "G70 [GeForce 7800 SLI]", DESKTOP },
	{ 0x0184, "NV18 [GeForce4 MX]", MOBILE },
	{ 0x0189, "NV18 [GeForce4 MX with AGP8X (Mac)]", MOBILE },
	{ 0x01d0, "G72 [GeForce 7350 LE]", DESKTOP },
	{ 0x01d2, "G72 [GeForce 7550 LE]", DESKTOP },
	{ 0x01d6, "G72M [GeForce Go 7200]", MOBILE },
	{ 0x0243, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x0246, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x0248, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x0249, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x024a, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x024b, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x024c, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x024d, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x024e, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x024f, "C51 PCI Express Bridge [?]", DESKTOP },
	{ 0x0252, "NV25 [GeForce4 Ti]", DESKTOP },
	{ 0x02e3, "G71 [GeForce 7900 GS]", DESKTOP },
	{ 0x0313, "NV31 [?]", DESKTOP },
	{ 0x0320, "NV34 [GeForce FX 5200]", DESKTOP },
	{ 0x0327, "NV34 [GeForce FX 5100]", DESKTOP },
	{ 0x038b, "G73 [GeForce 7650 GS]", DESKTOP },
	{ 0x03d6, "C61 [GeForce 7025 / nForce 630a]", DESKTOP },
	{ 0x0410, "G92 [GeForce GT 330]", DESKTOP },
	{ 0x042c, "G86 [GeForce 9400 GT]", DESKTOP },
	{ 0x0531, "C67 [GeForce 7150M / nForce 630M]", MOBILE },
	{ 0x0533, "C67 [GeForce 7000M / nForce 610M]", MOBILE },
	{ 0x05e3, "GT200b [GeForce GTX 285]", DESKTOP },
	{ 0x05e6, "GT200b [GeForce GTX 275]", DESKTOP },
	{ 0x05ea, "GT200 [GeForce GTX 260]", DESKTOP },
	{ 0x05eb, "GT200 [GeForce GTX 295]", DESKTOP },
	{ 0x05ff, "GT200GL [NVIDIA Quadro FX 3800]", DESKTOP },
	{ 0x0601, "G92 [GeForce 9800 GT]", DESKTOP },
	{ 0x0603, "G92 [GeForce GT 230]", DESKTOP },
	{ 0x0605, "G92 [GeForce 9800 GT]", DESKTOP },
	{ 0x0607, "G92 [GeForce GTS 240]", DESKTOP },
	{ 0x0608, "G92 [GeForce 9800M GTX]", MOBILE },
	{ 0x060a, "GT200 [GeForce GTX 280M]", MOBILE },
	{ 0x060b, "G92 [GeForce 9800M GT]", MOBILE },
	{ 0x060e, "G92 [GeForce 9850 E]", DESKTOP },
	{ 0x060f, "G92 [GeForce GTX 285M]", MOBILE },
	{ 0x0613, "G92 [GeForce 9800 GTX+]", DESKTOP },
	{ 0x0615, "G92 [GeForce GTS 250]", DESKTOP },
	{ 0x0617, "G92 [GeForce 9800M GTX]", MOBILE },
	{ 0x0618, "G92 [GeForce GTX 260M]", MOBILE },
	{ 0x0619, "G92GL [Quadro FX 4700 X2]", DESKTOP },
	{ 0x061b, "G92GL [Quadro VX 200]", DESKTOP },
	{ 0x061d, "G92 [Quadro FX 2800M]", MOBILE },
	{ 0x061e, "G92 [Quadro FX 3700M]", MOBILE },
	{ 0x061f, "G92 [Quadro FX 3800M]", MOBILE },
	{ 0x0621, "G94 [GeForce GT 230]", DESKTOP },
	{ 0x0625, "G94 [GeForce 9600 GSO 512]", DESKTOP },
	{ 0x0626, "G94 [GeForce GT 130]", DESKTOP },
	{ 0x0627, "G94 [GeForce GT 140]", DESKTOP },
	{ 0x0628, "G94 [GeForce 9800M GTS]", MOBILE },
	{ 0x062a, "G94 [GeForce 9700M GTS]", MOBILE },
	{ 0x062b, "G94 [GeForce 9800M GS]", MOBILE },
	{ 0x062c, "G94 [GeForce 9800M GTS]", MOBILE },
	{ 0x062d, "G94 [GeForce 9600 GT]", DESKTOP },
	{ 0x062e, "G94 [GeForce 9600 GT]", DESKTOP },
	{ 0x062f, "G94 [GeForce 9800 S]", DESKTOP },
	{ 0x0630, "G94 [GeForce 9700 S]", DESKTOP },
	{ 0x0631, "G94M [GeForce GTS 160M]", MOBILE },
	{ 0x0632, "G94M [GeForce GTS 150M]", MOBILE },
	{ 0x0633, "G94 [GeForce GT 220]", DESKTOP },
	{ 0x0635, "G94 [GeForce 9600 GSO]", DESKTOP },
	{ 0x0637, "G94 [GeForce 9600 GT]", DESKTOP },
	{ 0x0638, "G94 [Quadro FX 1800]", DESKTOP },
	{ 0x063a, "G94M [Quadro FX 2700M]", MOBILE },
	{ 0x0641, "G96 [GeForce 9400 GT]", DESKTOP },
	{ 0x0642, "G96 [GeForce 8400 GS]", DESKTOP },
	{ 0x0644, "G96 [GeForce 9500 GS]", DESKTOP },
	{ 0x0645, "G96 [GeForce 9500 GS]", DESKTOP },
	{ 0x0646, "G96 [GeForce GT 120]", DESKTOP },
	{ 0x064a, "G96 [GeForce 9700M GT]", MOBILE },
	{ 0x064c, "G96 [GeForce 9650M GT]", MOBILE },
	{ 0x064f, "G96 [GeForce 9600 S]", DESKTOP },
	{ 0x0650, "G96 [?]", DESKTOP },
	{ 0x0651, "G96 [GeForce G 110M]", MOBILE },
	{ 0x0652, "G96 [GeForce GT 130M]", MOBILE },
	{ 0x0653, "G96M [GeForce GT 120M]", MOBILE },
	{ 0x0654, "G96 [GeForce GT 220M]", MOBILE },
	{ 0x0655, "G96 [GeForce 9500 GS]", DESKTOP },
	{ 0x0656, "G96 [GeForce 9650 S]", DESKTOP },
	{ 0x0658, "G96 [Quadro FX 380]", DESKTOP },
	{ 0x0659, "G96 [Quadro FX 580]", DESKTOP },
	{ 0x065a, "G96 [Quadro FX 1700M]", MOBILE },
	{ 0x065b, "G96 [GeForce 9400 GT]", DESKTOP },
	{ 0x065c, "G96M [Quadro FX 770M]", MOBILE },
	{ 0x065f, "G96 [GeForce GT 210]", DESKTOP },
	{ 0x06a0, "GT214 [?]", DESKTOP },
	{ 0x06b0, "GT214 [?]", DESKTOP },
	{ 0x06c0, "GF100 [GeForce GTX 480]", DESKTOP },
	{ 0x06cd, "GF100 [GeForce GTX 470]", DESKTOP },
	{ 0x06d1, "GF100 [Tesla C2050]", DESKTOP },
	{ 0x07e0, "C73 [GeForce 7150 / nForce 630i]", DESKTOP },
	{ 0x07e1, "C73 [GeForce 7100 / nForce 630i]", DESKTOP },
	{ 0x07e2, "C73 [GeForce 7050 / nForce 630i]", DESKTOP },
	{ 0x07e3, "C73 [GeForce 7050 / nForce 610i]", DESKTOP },
	{ 0x07e5, "C73 [GeForce 7100 / nForce 620i]", DESKTOP },
	{ 0x0844, "C77 [GeForce 9100M G]", MOBILE },
	{ 0x0845, "C77 [GeForce 8200M G]", MOBILE },
	{ 0x0846, "C77 [GeForce 9200]", DESKTOP },
	{ 0x0847, "C78 [GeForce 9100]", DESKTOP },
	{ 0x0848, "C77 [GeForce 8300]", DESKTOP },
	{ 0x0849, "C77 [GeForce 8200]", DESKTOP },
	{ 0x084a, "C77 [nForce 730a]", DESKTOP },
	{ 0x084b, "C77 [GeForce 8200]", DESKTOP },
	{ 0x084c, "C77 [nForce 780a SLI]", DESKTOP },
	{ 0x084d, "C77 [nForce 750a SLI]", DESKTOP },
	{ 0x084f, "C77 [GeForce 8100 / nForce 720a]", DESKTOP },
	{ 0x0862, "C79 [GeForce 9400M G]", MOBILE },
	{ 0x0866, "C79 [GeForce 9400M G]", MOBILE },
	{ 0x0867, "C79 [GeForce 9400]", DESKTOP },
	{ 0x0868, "C79 [nForce 760i SLI]", DESKTOP },
	{ 0x086a, "C79 [GeForce 9400]", DESKTOP },
	{ 0x086c, "C79 [GeForce 9300 / nForce 730i]", DESKTOP },
	{ 0x086d, "C79 [GeForce 9200]", DESKTOP },
	{ 0x086e, "C79 [GeForce 9100M G]", MOBILE },
	{ 0x086f, "C79 [GeForce 9200M G]", MOBILE },
	{ 0x0870, "C79 [GeForce 9400M]", MOBILE },
	{ 0x0871, "C79 [GeForce 9200]", DESKTOP },
	{ 0x0872, "C79 [GeForce G102M]", MOBILE },
	{ 0x0873, "C79 [GeForce G102M]", MOBILE },
	{ 0x0874, "C79 [?]", DESKTOP },
	{ 0x0876, "C79 [GeForce 9400M]", MOBILE },
	{ 0x087a, "C79 [GeForce 9400]", DESKTOP },
	{ 0x087d, "C79 [?]", DESKTOP },
	{ 0x087e, "C79 [?]", DESKTOP },
	{ 0x087f, "C79 [?]", DESKTOP },
	{ 0x0a20, "GT216 [GeForce GT 220]", DESKTOP },
	{ 0x0a23, "GT218 [GeForce 210]", DESKTOP },
	{ 0x0a28, "GT216 [GeForce GT 230M]", MOBILE },
	{ 0x0a29, "GT216 [GeForce GT 330M]", MOBILE },
	{ 0x0a2a, "GT216 [GeForce GT 230M]", MOBILE },
	{ 0x0a2b, "GT216 [GeForce GT 330M]", MOBILE },
	{ 0x0a2c, "GT216 [NVS 5100M]", MOBILE },
	{ 0x0a2d, "GT216 [GeForce GT 320M]", MOBILE },
	{ 0x0a34, "GT216 [GeForce GT 240M]", MOBILE },
	{ 0x0a35, "GT216 [GeForce GT 325M]", MOBILE },
	{ 0x0a3c, "GT216 [Quadro FX 880M]", MOBILE },
	{ 0x0a60, "GT218 [GeForce G210]", DESKTOP },
	{ 0x0a62, "GT218 [GeForce 205]", DESKTOP },
	{ 0x0a63, "GT218 [GeForce 310]", DESKTOP },
	{ 0x0a64, "GT218 [?]", DESKTOP },
	{ 0x0a65, "GT218 [GeForce 210]", DESKTOP },
	{ 0x0a66, "GT218 [GeForce 310]", DESKTOP },
	{ 0x0a68, "G98M [GeForce G105M]", MOBILE },
	{ 0x0a69, "G98M [GeForce G105M]", MOBILE },
	{ 0x0a6a, "GT218 [NVS 2100M]", MOBILE },
	{ 0x0a6c, "GT218 [NVS 3100M]", MOBILE },
	{ 0x0a6e, "GT218 [GeForce 305M]", MOBILE },
	{ 0x0a6f, "GT218 [?]", DESKTOP },
	{ 0x0a70, "GT218 [GeForce 310M]", MOBILE },
	{ 0x0a71, "GT218 [GeForce 305M]", MOBILE },
	{ 0x0a72, "GT218 [GeForce 310M]", MOBILE },
	{ 0x0a73, "GT218 [GeForce 305M]", MOBILE },
	{ 0x0a74, "GT218 [GeForce G210M]", MOBILE },
	{ 0x0a75, "GT218 [GeForce 310M]", MOBILE },
	{ 0x0a78, "GT218GL [Quadro FX 380 LP]", DESKTOP },
	{ 0x0a7c, "GT218 [Quadro FX 380M]", MOBILE },
	{ 0x0ca0, "GT215 [GeForce GT 330]", DESKTOP },
	{ 0x0ca2, "GT215 [GeForce GT 320]", DESKTOP },
	{ 0x0ca3, "GT215 [GeForce GT 240]", DESKTOP },
	{ 0x0ca4, "GT215 [GeForce GT 340]", DESKTOP },
	{ 0x0ca5, "GT215 [?]", DESKTOP },
	{ 0x0ca7, "GT215 [GeForce GT 330]", DESKTOP },
	{ 0x0ca8, "GT215 [GeForce GTS 260M]", MOBILE },
	{ 0x0ca9, "GT215 [GeForce GTS 250M]", MOBILE },
	{ 0x0cad, "GT215 [?]", DESKTOP },
	{ 0x0caf, "GT215 [GeForce GT 335M]", MOBILE },
	{ 0x0cb0, "GT215 [GeForce GTS 350M]", MOBILE },
	{ 0x0cb1, "GT215 [GeForce GTS 360M]", MOBILE },
	{ 0x0cbc, "GT215 [Quadro FX 1800M]", MOBILE },


	{ 0, NULL, UNKNOWN }
};

const char *get_card_name(int device_id, gpu_type *gpu)
{
	struct pci_ids *nv_ids = (struct pci_ids*)ids;

	while(nv_ids->id != 0)
	{
		if(nv_ids->id == device_id)
		{
			*gpu = nv_ids->gpu;
			return nv_ids->name;
		}

		nv_ids++;
	}

	/* if !found */
	*gpu = UNKNOWN;
	return "Unknown Nvidia card";
}

/* Internal gpu architecture function which sets
   a device to a specific architecture. This architecture
   doesn't have to be the real architecture. It is mainly
   used to choose codepaths inside nvclock.
*/
int get_gpu_arch(int device_id)
{
	int arch;
	switch(device_id & 0xfff0)
	{
	case 0x0020:
		arch = NV5;
		break;
	case 0x0100:
	case 0x0110:
	case 0x0150:
	case 0x01a0:
		arch = NV10;
		break;
	case 0x0170:
	case 0x0180:
	case 0x01f0:
		arch = NV17;
		break;
	case 0x0200:
		arch = NV20;
		break;
	case 0x0250:
	case 0x0280:
	case 0x0320:	/* We don't treat the FX5200/FX5500 as FX cards */
		arch = NV25;
		break;
	case 0x0300:
		arch = NV30;
		break;
	case 0x0330:
		arch = NV35; /* Similar to NV30 but fanspeed stuff works differently */
		break;
		/* Give a seperate arch to FX5600/FX5700 cards as they need different code than other FX cards */
	case 0x0310:
	case 0x0340:
		arch = NV31;
		break;
	case 0x0040:
	case 0x0120:
	case 0x0130:
	case 0x0210:
	case 0x0230:
		arch = NV40;
		break;
	case 0x00c0:
		arch = NV41;
		break;
	case 0x0140:
		arch = NV43; /* Similar to NV40 but with different fanspeed code */
		break;
	case 0x0160:
	case 0x0220:
		arch = NV44;
		break;
	case 0x01d0:
		arch = NV46;
		break;
	case 0x0090:
		arch = NV47;
		break;
	case 0x0290:
		arch = NV49; /* 7900 */
		break;
	case 0x0380:
	case 0x0390:
		arch = NV4B; /* 7600 */
		break;
	case 0x0190:
		arch = NV50; /* 8800 'NV50 / G80' */
		break;
	case 0x0400: /* 8600 'G84' */
		arch = G84;
		break;
	case 0x0420: /* 8500 'G86' */
		arch = G86;
		break;
	case 0x05e0: /* GT2x0 */
	case 0x05f0: /* GT2x0 */
	case 0x06a0:
	case 0x06b0:
	case 0x06c0:
	case 0x06d0:
	case 0x0a20:
	case 0x0a30:
	case 0x0a60:
	case 0x0a70:
	case 0x0ca0:
	case 0x0cb0:
		arch = GT200;
		break;
	case 0x06e0: /* G98 */
	case 0x06f0: /* G98 */
	case 0x0860: /* C79 */
	case 0x07e0: /* C73 */
	case 0x0840: /* C77 */
	case 0x0870: /* C79 */
		arch = G86;
		break;
	case 0x0410:
	case 0x0600: /* G92 */
	case 0x0610: /* G92 */
		arch = G92;
		break;
	case 0x0620: /* 9600GT 'G94' */
	case 0x0630:
		arch = G94;
		break;
	case 0x0640: /* 9500GT */
	case 0x0650:
		arch = G96;
		break;
	case 0x0240:
	case 0x03d0: /* not sure if this is a C51 too */
	case 0x0530: /* not sure if the 70xx is C51 too */
		arch = C51;
		break;
	case 0x02e0:
	case 0x00f0:
		/* The code above doesn't work for pci-express cards as multiple architectures share one id-range */
		switch(device_id)
		{
		case 0x00f0: /* 6800 */
		case 0x00f9: /* 6800Ultra */
			arch = NV40;
			break;
		case 0x00f6: /* 6800GS/XT */
			arch = NV41;
			break;
		case 0x00f1: /* 6600/6600GT */
		case 0x00f2: /* 6600GT */
		case 0x00f3: /* 6200 */
		case 0x00f4: /* 6600LE */
			arch = NV43;
			break;
		case 0x00f5: /* 7800GS */
			arch = NV47;
			break;
		case 0x00fa: /* PCX5700 */
			arch = NV31;
			break;
		case 0x00f8: /* QuadroFX 3400 */
		case 0x00fb: /* PCX5900 */
			arch = NV35;
			break;
		case 0x00fc: /* PCX5300 */
		case 0x00fd: /* Quadro NVS280/FX330, FX5200 based? */
		case 0x00ff: /* PCX4300 */
			arch = NV25;
			break;
		case 0x00fe: /* Quadro 1300, has the same id as a FX3000 */
			arch = NV35;
			break;
		case 0x02e0: /* Geforce 7600GT AGP (at least Leadtek uses this id) */
		case 0x02e1: /* Geforce 7600GS AGP (at least BFG uses this id) */
		case 0x02e2: /* Geforce 7300GT AGP (at least a Galaxy 7300GT uses this id) */
			arch = NV4B;
			break;
		case 0x02e4: /* Geforce 7950 GT AGP */
			arch = NV49;
			break;
		}
		break;
	default:
		arch = UNKNOWN;
	}
	return arch;
}

/* Receive the real gpu architecture */
static short get_gpu_architecture()
{
	return (nv_read_pmc(NV_PMC_BOOT_0) >> 20) & 0xff;
}

/* Receive the gpu revision */
static short get_gpu_revision()
{
	return nv_read_pmc(NV_PMC_BOOT_0) & NV_PMC_BOOT_0_REVISION_MASK;
}

/* Retrieve the 'real' PCI id from the card */
static short get_gpu_pci_id()
{
	return nv_read_pbus16(PCI_DEVICE_ID);
}

/* Retrieve the pci subvendor id */
static short get_pci_subvendor_id()
{
	return nv_read_pbus16(PCI_SUBSYSTEM_VENDOR_ID);
}

static int set_gpu_pci_id(short id)
{
	if(nv_card->arch & (NV10 | NV20))
	{
		/* The first two bits of the pci id can be changed. They are stored in bit 13-12 of PEXTDEV_BOOT0 */
		int pextdev_boot0 = nv_card->PEXTDEV[0x0/4] & ~(0x3 << 12);
		/* Only the first 2 bits can be changed on these GPUs */
		if(id > 3)
			return 0;

		nv_card->PEXTDEV[0x0/4] = pextdev_boot0 | ((id & 0x3) << 12);
		nv_card->device_id = get_gpu_pci_id();
		nv_card->card_name = (char*)get_card_name(nv_card->device_id, &nv_card->gpu);
		return 1;
	}
	/* Don't allow modding on cards using bridges (0xf*)! */
	else if((nv_card->arch & (NV17 | NV25 | NV3X | NV4X)) && ((nv_card->device_id & 0xfff0) != 0xf0))
	{
		/* The first four bits of the pci id can be changed. The first two bits are stored in bit 13-12 of PEXTDEV_BOOT0, bit 3 and 4 are stored in bit 21-20  */
		int pextdev_boot0 = nv_card->PEXTDEV[0x0/4] & ~((0x3 << 20) | (0x3 << 12));
		/* The first 4 bits can be changed */
		if(id > 16)
			return 0;
		
		/* On various NV4x cards the quadro capability bit in PBUS_DEBUG1 is locked. It can be unlocked by setting the first bit in 0xc020/0xc028 */
		nv_card->PMC[0xc020/4] = 1;
		nv_card->PMC[0xc028/4] = 1;

		nv_card->PEXTDEV[0x0/4] = pextdev_boot0 | (((id>>2) & 0x3) << 20) | ((id & 0x3) << 12);
		nv_card->device_id = get_gpu_pci_id();
		nv_card->card_name = (char*)get_card_name(nv_card->device_id, &nv_card->gpu);
		return 1;
	}
	return 0;
}

/* Function to read a single byte from pci configuration space */
static void read_byte(int offset, unsigned char *data)
{
	/* The original plan was to read the PCI configuration directly from registers 0x1800 and upwards
	   from the card itself. Although this is a fully correct way, it doesn't work for some cards using
	   a PCI-Express -> AGP bridge. If I would read the registers from the card they would include PCI-Express
	   as one of the capabilities. Reading using the "normal" way results in AGP as one of the capabilities.
	   To correctly show that a card uses AGP we need to read the modded config space.
	*/
	*data = pciReadLong(nv_card->devbusfn,offset) & 0xff;
}

/* Check the videocard for a certain PCI capability like AGP/PCI-Express/PowerManagement.
   If a certain capability is supported return the position of the cap pointer. 
*/
static int pci_find_capability(unsigned char cap)
{
	unsigned char pos, id;

	read_byte(PCI_CAPABILITY_LIST, &pos);

	while(pos >= 0x40)
	{
		pos &= ~3;
		read_byte(pos + PCI_CAP_LIST_ID, &id);
		if(id == 0xff)
			break;
		if(id == cap)
			return pos; /* Return the position of the cap pointer */

		read_byte(pos + PCI_CAP_LIST_NEXT, &pos);
	}
	return 0;
}

/* Check the videocard for a certain PCI capability like AGP/PCI-Express/PowerManagement.
   If a certain capability is supported return the position of the cap pointer. 
*/
static int nv_pci_find_capability(unsigned char cap)
{
	unsigned char pos, id;

	pos = nv_read_pbus8(PCI_CAPABILITY_LIST);

	while(pos >= 0x40)
	{
		pos &= ~3;
		id = nv_read_pbus8(pos + PCI_CAP_LIST_ID);
		if(id == 0xff)
			break;
		if(id == cap)
			return pos; /* Return the position of the cap pointer */

		pos = nv_read_pbus8(pos + PCI_CAP_LIST_NEXT);
	}
	return 0;
}

static char* get_bus_type()
{
	/* The pci header contains lots of information about a device like
	   what type of device it is, who the vendor is and so on. It also
	   contains a list of capabilities. Things like AGP, power management,
	   PCI-X and PCI-Express are considered capabilities. We could check
	   these capabilities to find out if the card is AGP or PCI-Express.
       
	   Reading the bus type from the pci header would be a nice way but
	   unfortunately there are some issues. One way to do the reading
	   is to read the information directly from the card (from PMC).
	   This doesn't work as some PCI-Express boards (6600GT) actually use
	   PCI-Express GPUs connected to some bridge chip on AGP boards (same device id!).
	   If you read directly from the card it will advertise PCI-Express instead of AGP.
	   There is also another way to read the pci header for instance on Linux
	   using from files in /proc/bus/pci but non-root users can only read
	   a small part of the file. Most of the time the info we need isn't in
	   the readable part. Further there are also some early PCI-Express boards (GeforcePCX)
	   that contain bridge chips to turn AGP GPUs into PCI-Express ones.
       
	   Currently we will return PCI-Express on GeforcePCX board under the valid
	   assumption that there are no AGP boards with the same device id. Further
	   it seems that 'low' device ids are for PCI-Express->AGP while the higher ones
	   are for AGP->PCI-Express, so for the lower ones (6200/6600/6800) we will return AGP
	   and for the higher ones PCI-Express. A nicer way would be to read all this stuff from
	   the pci header but as explained that can't be done at the moment.
	*/
	switch(nv_card->device_id)
	{
	case 0xf0: /* 6800 */
	case 0xf1: /* 6600GT */
	case 0xf2: /* 6600 */
	case 0xf3: /* 6200 */
	case 0xf5: /* 6800GS/XT */
	case 0xf6: /* 7800GS */
	case 0x2e0: /* 7600GT */
	case 0x2e1: /* 7600GS */
	case 0x2e2: /* 7300GT */
		return "AGP (BR02)"; /* We return something different from AGP for now as we don't want to show the AGP tab */
	case 0xf8: /* Quadro FX3400 */
	case 0xf9: /* Geforce 6800 series */
	case 0xfa: /* PCX5500 */
	case 0xfb: /* PCX5900 */
	case 0xfc: /* Quadro FX330*/
	case 0xfd: /* PCX5500 */
	case 0xfe: /* Quadro 1300 */
	case 0xff: /* PCX4300 */
		return "PCI-Express (BR02)";
	}

	if(nv_pci_find_capability(PCI_CAP_ID_EXP))
		return "PCI-Express";
	else if(nv_pci_find_capability(PCI_CAP_ID_AGP))
		return "AGP";
	else
		return "PCI";
}

/* Needs better bus checks .. return a string ?*/
static short get_agp_bus_rate()
{
	int agp_capptr, agp_rate, agp_status;

	agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	agp_rate = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND) & PCI_AGP_STATUS_RATE_MASK;

	/* If true, the user has AGP8x support */
	if(agp_status & PCI_AGP_STATUS_RATE_8X_SUPPORT)
	{
		agp_rate <<= PCI_AGP_STATUS_RATE_8X_SHIFT;
	}
	return agp_rate;
}

static char* get_agp_fw_status()
{
	int agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	unsigned int agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	unsigned int agp_command = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND);

	/* Check if Fast Writes is supported by the hostbridge */
	if(agp_status & PCI_AGP_STATUS_FW)
		return (agp_command & PCI_AGP_COMMAND_FW) ? "Enabled" : "Disabled";
	else
		return "Unsupported";
}

static char* get_agp_sba_status()
{
	int agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	unsigned int agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	unsigned int agp_command = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND);

	/* Check if Sideband Addressing is supported by the hostbridge */
	if(agp_status & PCI_AGP_STATUS_SBA)
		return (agp_command & PCI_AGP_COMMAND_SBA) ? "Enabled" : "Disabled";
	else
		return "Unsupported";
}

static char* get_agp_status()
{
	int agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	unsigned int agp_command = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND);
	return (agp_command & PCI_AGP_COMMAND_AGP) ? "Enabled" : "Disabled";
}

static char* get_agp_supported_rates()
{
	int agp_capptr, agp_rates, agp_status, i;
	static char *rate;

	agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	agp_rates = agp_status & PCI_AGP_STATUS_RATE_MASK;

	/* If true, the user has AGP8x support */
	if(agp_status & PCI_AGP_STATUS_RATE_8X_SUPPORT)
	{
		agp_rates <<= PCI_AGP_STATUS_RATE_8X_SHIFT;
	}

	rate = (char*)calloc(1, sizeof(char));

	for(i=1; i <= 8; i*=2)
	{
		if(agp_rates & i)
		{
			char *temp = (char*)malloc(4 * sizeof(char));
			sprintf(temp, "%dX ", i);
			rate = (char*)realloc(rate, strlen(rate)+4);
			rate = strcat(rate, temp);
			free(temp);
		}
	}

	return rate;
}

static short get_pcie_bus_rate()
{
	int pcie_rate, pcie_status_reg;

	pcie_status_reg = nv_pci_find_capability(PCI_CAP_ID_EXP);
	if(pcie_status_reg != 0 )
	{
		pcie_rate = (nv_read_pbus16(pcie_status_reg + PCIE_LINKSTATUS) & PCIE_LINK_SPEED_MASK) >> PCIE_LINK_SPEED_SHIFT;
		return pcie_rate;
	}
	return 0;
}

static short get_pcie_max_bus_rate()
{
	int pcie_rate, pcie_status_reg;

	pcie_status_reg = nv_pci_find_capability(PCI_CAP_ID_EXP);
	if(pcie_status_reg != 0 )
	{
		pcie_rate = (nv_read_pbus16(pcie_status_reg + PCIE_LINKCAP) & PCIE_LINK_SPEED_MASK) >> PCIE_LINK_SPEED_SHIFT;

		return pcie_rate;
	}
	return 0;
}

static short get_memory_width()
{
	/* Nforce / Nforce2 */
	if((nv_card->device_id == 0x1a0) || (nv_card->device_id == 0x1f0))
		return 64;
	/* GeforceFX cards (except for FX5200) need a different check */
	/* What to do with NV40 cards ? */
	else if(nv_card->arch & NV3X)
	{
		/* I got this info from the rivatuner forum. On the forum
		 *  is a thread containing register dumps from lots of cards.
		 *  It might not be 100% correct but it is better than a pci id check */
		switch(nv_card->PFB[0x200/4] & 0x7)
		{
			/* 64bit FX5600 */
		case 0x1:
			return 64;
			/* 128bit FX5800 */
		case 0x3:
			return 128;
			/* 128bit FX5600, FX5700 */
		case 0x5:
			return 128;
			/* 256bit FX5900 */
		case 0x7:
			return 256;
		}
	}
	else if(nv_card->arch == NV44)
	{
		return 64; /* For now return 64; (Turbocache cards) */
	}
	else if(nv_card->arch & NV4X)
	{
		/* Memory bandwith detection for nv40 but not sure if it is correct, it is atleast better than nothing */
		switch(nv_card->PFB[0x200/4] & 0x7)
		{
			/* 128bit 6600GT */
		case 0x1:
			return 128;
			/* 256bit 6800 */
		case 0x3:
			return 256;
		default:
			return 128;
		}
	}
	else if(nv_card->arch & NV5X)
	{
		/* On Geforce 8800GTS/GTX and 8600GT/GTS cards the memory bandwith is proportional to the number of ROPs * 16.
		 *  In case of the 8500 this isn't the case, there the size is just 128 where there are 4 ROPs.
		 *  So for now use the number of ROPs as a meassure for the bus width.
		 */
		char rmask, rmask_default;
		switch(nv_card->get_rop_units(&rmask, &rmask_default))
		{
		case 32: /* Geforce GTX280 */
			return 512;
		case 28: /* Geforce GTX260 */
			return 448;
		case 24: /* 8800GTX */
			return 384;
		case 20: /* 8800GTS */
			return 320;
		case 16: /* 8800GT */
			return 256;
		case 12: /* 8800GS */
			return 192;
		case 8: /* 8600GT/GTS */
		case 4: /* 8500GT; 8400GS boards use the same core and offer 64-bit, how to handle this? */
			return 128;
		case 2: /* 8300GS */
			return 64;
		}
	}
	/* Generic algorithm for cards up to the Geforce4 */
	return (nv_card->PEXTDEV[0x0/4] & 0x17) ? 128 : 64;
}

static char* get_memory_type()
{
	/* Nforce / Nforce2 */
	if((nv_card->device_id == 0x1a0) || (nv_card->device_id == 0x1f0))
		return ((pciReadLong(0x1, 0x7c) >> 12) & 0x1) ? "DDR" : "SDR";
	else if(nv_card->arch & (NV2X | NV3X))
	{
		/* Based on statistics found on the rivatuner forum, the first two bytes of
		 * register 0x1218 of NV2X/NV3X boards, contains "0x0001 or 0x0101" in case of DDR memory and "0x0301" for DDR2.
		 */
		return (((nv_card->PMC[0x1218/4] >> 8) & 0x3) == 0x3) ? "DDR2" : "DDR";
	}
	else if(nv_card->arch & (NV4X))
	{
		/* On Geforce6/7 cards 0x100474 (PFB 0x474) can be used to distinguish between DDR and DDR3. 
		 * Note these values are based on the bios and it was noted that for instance bits in this register differ.
		 * In case of DDR3 the first byte contains 0x4 while in case of DDR it contains 0x1.
		 */
		return (nv_card->PFB[0x474/4] & 0x4) ? "DDR3" : "DDR";
	}
	else if(nv_card->arch & (NV5X))
	{
		/* For now use 0x100218 (PFB 0x218) to distinguish between DDR2 and DDR3. The contents of this
		 *  register differs between a 8500GT (DDR2) and 8600GTS/8800GTS (DDR3) according to the bios.
		 *  FIXME: use a better register
		 */
		return (nv_card->PFB[0x218/4] & 0x1000000) ? "DDR3" : "DDR2";
	}
	else
		/* TNT, Geforce1/2/4MX */
		return (nv_card->PFB[0x200/4] & 0x01) ? "DDR" : "SDR";
}

static short get_memory_size()
{
	short memory_size;

	/* If the card is something TNT based the calculation of the memory is different. */
	if(nv_card->arch == NV5)
	{
		if(nv_card->PFB[0x0/4] & 0x100)
			memory_size = ((nv_card->PFB[0x0/4] >> 12) & 0xf)*2+2;
		else
		{
			switch(nv_card->PFB[0x0/4] & 0x3)
			{
			case 0:
				memory_size = 32;
				break;
			case 1:
				memory_size = 4;
				break;
			case 2:
				memory_size = 8;
				break;
			case 3:
				memory_size = 16;
				break;
			default:
				memory_size = 16;
				break;
			}
		}
	}
	/* Nforce 1 */
	else if(nv_card->device_id == 0x1a0)
	{
		int32_t temp = pciReadLong(0x1, 0x7c);
		memory_size = ((temp >> 6) & 0x31) + 1;
	}
	/* Nforce2 */
	else if(nv_card->device_id == 0x1f0)
	{
		int32_t temp = pciReadLong(0x1, 0x84);
		memory_size = ((temp >> 4) & 0x127) + 1;
	}
	/* Memory calculation for geforce cards or better.*/
	else
	{
		/* The code below is needed to show more than 256MB of memory
		   but I'm not sure if 0xfff is safe for pre-geforcefx cards.
		   There's no clean way right now to use 0xff for those old cards
		   as currently the FX5200/FX5500 (which support 256MB) use the
		   pre-geforcefx backend.
		*/
		memory_size = (nv_card->PFB[0x20c/4] >> 20) & 0xfff;
	}

	return memory_size;
}

/* Print various GPU registers for debugging purposes */
static void get_debug_info()
{
	printf("--- %s GPU registers ---\n", nv_card->card_name);
	printf("NV_PMC_BOOT_0 (0x0): %08x\n", nv_card->PMC[0]);
	printf("NV_PBUS_DEBUG_0 (0x1080): %08x\n", nv_card->PMC[0x1080/4]);
	printf("NV_PBUS_DEBUG_1 (0x1084): %08x\n", nv_card->PMC[0x1084/4]);
	printf("NV_PBUS_DEBUG_2 (0x1088): %08x\n", nv_card->PMC[0x1088/4]);
	printf("NV_PBUS_DEBUG_3 (0x108c): %08x\n", nv_card->PMC[0x108c/4]);
	printf("NV_10F0 (0x10f0): %08x\n", nv_card->PMC[0x10f0/4]);
	printf("NV_1540 (0x1540): %08x\n", nv_card->PMC[0x1540/4]);
	printf("NV_15B0 (0x15b0): %08x\n", nv_card->PMC[0x15b0/4]);
	printf("NV_15B4 (0x15b4): %08x\n", nv_card->PMC[0x15b4/4]);
	printf("NV_15B8 (0x15b8): %08x\n", nv_card->PMC[0x15b8/4]);
	printf("NV_15F0 (0x15f0): %08x\n", nv_card->PMC[0x15f0/4]);
	printf("NV_15F4 (0x15f4): %08x\n", nv_card->PMC[0x15f4/4]);
	printf("NV_15F8 (0x15f8): %08x\n", nv_card->PMC[0x15f8/4]);
	printf("NV_PBUS_PCI_0 (0x1800): %08x\n", nv_read_pbus(PCI_VENDOR_ID));
	printf("NV_PBUS_PCI_0 (0x182c): %08x\n", nv_read_pbus(PCI_SUBSYSTEM_VENDOR_ID));

	if(nv_card->arch & (NV4X | NV5X))
	{
		printf("NV_C010 (0xc010): %08x\n", nv_card->PMC[0xc010/4]);
		printf("NV_C014 (0xc014): %08x\n", nv_card->PMC[0xc014/4]);
		printf("NV_C018 (0xc018): %08x\n", nv_card->PMC[0xc018/4]);
		printf("NV_C01C (0xc01c): %08x\n", nv_card->PMC[0xc01c/4]);
		printf("NV_C020 (0xc020): %08x\n", nv_card->PMC[0xc020/4]);
		printf("NV_C024 (0xc024): %08x\n", nv_card->PMC[0xc024/4]);
		printf("NV_C028 (0xc028): %08x\n", nv_card->PMC[0xc028/4]);
		printf("NV_C02C (0xc02c): %08x\n", nv_card->PMC[0xc02c/4]);
		printf("NV_C040 (0xc040): %08x\n", nv_card->PMC[0xc040/4]);
		printf("NV_4000 (0x4000): %08x\n", nv_card->PMC[0x4000/4]);
		printf("NV_4004 (0x4004): %08x\n", nv_card->PMC[0x4004/4]);
		printf("NV_4008 (0x4008): %08x\n", nv_card->PMC[0x4008/4]);
		printf("NV_400C (0x400c): %08x\n", nv_card->PMC[0x400c/4]);
		printf("NV_4010 (0x4010): %08x\n", nv_card->PMC[0x4010/4]);
		printf("NV_4014 (0x4014): %08x\n", nv_card->PMC[0x4014/4]);
		printf("NV_4018 (0x4018): %08x\n", nv_card->PMC[0x4018/4]);
		printf("NV_401C (0x401c): %08x\n", nv_card->PMC[0x401c/4]);
		printf("NV_4020 (0x4020): %08x\n", nv_card->PMC[0x4020/4]);
		printf("NV_4024 (0x4024): %08x\n", nv_card->PMC[0x4024/4]);
		printf("NV_4028 (0x4028): %08x\n", nv_card->PMC[0x4028/4]);
		printf("NV_402C (0x402c): %08x\n", nv_card->PMC[0x402c/4]);
		printf("NV_4030 (0x4030): %08x\n", nv_card->PMC[0x4030/4]);
		printf("NV_4034 (0x4034): %08x\n", nv_card->PMC[0x4034/4]);
		printf("NV_4038 (0x4038): %08x\n", nv_card->PMC[0x4038/4]);
		printf("NV_403C (0x403c): %08x\n", nv_card->PMC[0x403c/4]);
		printf("NV_4040 (0x4040): %08x\n", nv_card->PMC[0x4040/4]);
		printf("NV_4044 (0x4044): %08x\n", nv_card->PMC[0x4044/4]);
		printf("NV_4048 (0x4048): %08x\n", nv_card->PMC[0x4048/4]);
		printf("NV_404C (0x404c): %08x\n", nv_card->PMC[0x404c/4]);
		printf("NV_4050 (0x4050): %08x\n", nv_card->PMC[0x4050/4]);
		printf("NV_4054 (0x4054): %08x\n", nv_card->PMC[0x4054/4]);
		printf("NV_4058 (0x4058): %08x\n", nv_card->PMC[0x4058/4]);
		printf("NV_405C (0x405c): %08x\n", nv_card->PMC[0x405c/4]);
		printf("NV_4060 (0x4060): %08x\n", nv_card->PMC[0x4060/4]);
	}
	if(nv_card->arch & NV5X)
	{
		printf("NV_E100 (0xe100): %08x\n", nv_card->PMC[0xe100/4]);
		printf("NV_E114 (0xe114): %08x\n", nv_card->PMC[0xe114/4]);
		printf("NV_E118 (0xe118): %08x\n", nv_card->PMC[0xe118/4]);
		printf("NV_E11C (0xe11c): %08x\n", nv_card->PMC[0xe11c/4]);
		printf("NV_E120 (0xe120): %08x\n", nv_card->PMC[0xe120/4]);
		printf("NV_E300 (0xe300): %08x\n", nv_card->PMC[0xe300/4]);
		printf("NV_20008 (0x20008): %08x\n", nv_card->PMC[0x20008/4]);
		printf("NV_20400 (0x20400): %08x\n", nv_card->PMC[0x20400/4]);
		printf("NV_PDISPLAY_SOR0_REGS_BRIGHTNESS(%x): %08x\n", NV_PDISPLAY_SOR0_REGS_BRIGHTNESS, nv_card->PDISPLAY[NV_PDISPLAY_SOR0_REGS_BRIGHTNESS/4]);
	}

	printf("NV_PFB_CFG0 (0x100200): %08x\n", nv_card->PFB[0x200/4]);
	printf("NV_PFB_CFG0 (0x100204): %08x\n", nv_card->PFB[0x204/4]);
	printf("NV_PFB_CFG0 (0x100208): %08x\n", nv_card->PFB[0x208/4]);
	printf("NV_PFB_CFG0 (0x10020c): %08x\n", nv_card->PFB[0x20c/4]);
	printf("NV_PFB_218  (0x100218): %08x\n", nv_card->PFB[0x218/4]);
	printf("NV_PFB_TIMING0 (0x100220): %08x\n", nv_card->PFB[0x220/4]);
	printf("NV_PFB_TIMING1 (0x100224): %08x\n", nv_card->PFB[0x224/4]);
	printf("NV_PFB_TIMING2 (0x100228): %08x\n", nv_card->PFB[0x228/4]);
	printf("NV_PFB_474     (0x100474): %08x\n", nv_card->PFB[0x474/4]);
	printf("NV_PEXTDEV_BOOT_0 (0x101000): %08x\n", nv_card->PEXTDEV[0x0/4]);
	printf("NV_NVPLL_COEFF_A (0x680500): %08x\n", nv_card->PRAMDAC[0x500/4]);
	printf("NV_MPLL_COEFF_A (0x680504): %08x\n", nv_card->PRAMDAC[0x504/4]);
	printf("NV_VPLL_COEFF (0x680508): %08x\n", nv_card->PRAMDAC[0x508/4]);
	printf("NV_PLL_COEFF_SELECT (0x68050c): %08x\n", nv_card->PRAMDAC[0x50c/4]);
	printf("NV_NVPLL_COEFF_B (0x680570: %08x\n", nv_card->PRAMDAC[0x570/4]);
	printf("NV_MPLL_COEFF_B (0x680574: %08x\n", nv_card->PRAMDAC[0x574/4]);

	/* The builtin tvout encoder is available on Geforce4MX/TI and all other GPUs upto NV3x/NV4x.
	 * The registers are somewhere else on Geforce8 cards. There is a difference between the encoders
	 * on the difference cards but I'm not sure which apart from more features like the addition of
	 * component on the Geforce6 */
	if(nv_card->arch & (NV17 | NV25 | NV3X | NV4X))
	{
		int index=0;
		printf("--- TVOut regs ---\n");
		printf("0xd200: 0x%08x\n", nv_card->PMC[0xd200/4]); /* bit27-24 flickering (?) */
		printf("0xd204: 0x%08x\n", nv_card->PMC[0xd204/4]);
		printf("0xd208: 0x%08x\n", nv_card->PMC[0xd208/4]); /* Overscan */
		printf("0xd20c: 0x%08x\n", nv_card->PMC[0xd20c/4]);
		printf("0xd210: 0x%08x\n", nv_card->PMC[0xd210/4]); /* bit 23-8 contain the horizontal resolution */
		printf("0xd214: 0x%08x\n", nv_card->PMC[0xd214/4]); /* bit 23-8 contain the vertical resolution */
		printf("0xd218: 0x%08x\n", nv_card->PMC[0xd218/4]); /* bit31 = sign bit; bit16 and up can be used for horizontal positioning */

		printf("0xd21c: 0x%08x\n", nv_card->PMC[0xd21c/4]); /* bit31 = sign bit; bit16 and up can be used for vertical positioning */

		printf("0xd228: 0x%08x\n", nv_card->PMC[0xd228/4]);  /* is this some clock signal?? */
		printf("0xd22c: 0x%08x\n", nv_card->PMC[0xd22c/4]);
		
		printf("0xd230: 0x%08x\n", nv_card->PMC[0xd230/4]);
		printf("0xd304: 0x%08x\n", nv_card->PMC[0xd304/4]); /* bit 25-16 hscaler (PAL 720, NTSC 720) */
		printf("0xd508: 0x%08x\n", nv_card->PMC[0xd508/4]); /* bit 25-26 vscalar (PAL 288, NTSC 240) */
		printf("0xd600: 0x%08x\n", nv_card->PMC[0xd600/4]);
		printf("0xd604: 0x%08x\n", nv_card->PMC[0xd604/4]);
		printf("0xd608: 0x%08x\n", nv_card->PMC[0xd608/4]);

		/* Register 0xd220/0xd224 form a index/data register pair
		 * - 0x7 = bit4:0 connector type; bit2 s-video, bit2-0 empty: composite?
		 * - 0xe = bit7:0 tv system; ntscm (japan) 0x2; palb/d/g 0x4; palm/n/ntsc 0xc; is this correct? 
		 * - 0x22 = tv saturation
		 * - 0x25 = tv hue
		 * how many indices exist ?
		 */
		for(index=0; index < 0x80; index++)
		{
			nv_card->PMC[0xd220/4] = index;
			printf("index 0x%x: %02x\n", index, nv_card->PMC[0xd224/4]);
		}
	}
}


void info_init(void)
{
	nv_card->get_bus_type = get_bus_type;

	/* Set the pci id again as the detected id might not be accurate in case of pci id modding. The OS doesn't allways update the id while it really changed! Only do it for cards without bridges (device_id != 0xf* and 0x2e*
	 * Don't use this for NV50 as the location of the pci config header has changed to an unknown position.
	 */
	if(((nv_card->device_id & 0xfff0) != 0xf0) && ((nv_card->device_id & 0xfff0) != 0x2e0) && !(nv_card->arch & NV5X))
	{
		nv_card->device_id = get_gpu_pci_id(); 
		nv_card->card_name = (char*)get_card_name(nv_card->device_id, &nv_card->gpu);
	}

	/* Set the pci subvendor id */
	nv_card->subvendor_id = get_pci_subvendor_id();

	/* gpu arch/revision */
	nv_card->get_gpu_architecture = get_gpu_architecture;
	nv_card->get_gpu_revision = get_gpu_revision;

	/* Allow modding on all Geforce cards except for ones using bridges */
	if((nv_card->arch & (NV1X | NV2X | NV3X | NV4X)) && ((nv_card->device_id & 0xfff0) != 0xf0))
	{
		nv_card->caps |= GPU_ID_MODDING;
		nv_card->set_gpu_pci_id = set_gpu_pci_id;
	}
	else
		nv_card->set_gpu_pci_id = NULL;

	/* Check if card is a native AGP one and not using a bridge chip else we can't use the code below */
	if(strcmp(nv_card->get_bus_type(), "AGP") == 0)
	{
		nv_card->get_bus_rate = get_agp_bus_rate;
		nv_card->get_agp_status = get_agp_status;
		nv_card->get_agp_fw_status = get_agp_fw_status;
		nv_card->get_agp_sba_status = get_agp_sba_status;
		nv_card->get_agp_supported_rates = get_agp_supported_rates;
	}
	/* Check if card is a native PCI-Express one and not using a bridge chip else we can't use the code below */
	else if(strcmp(nv_card->get_bus_type(), "PCI-Express") == 0)
	{
		nv_card->get_bus_rate = get_pcie_bus_rate;
		nv_card->get_pcie_max_bus_rate = get_pcie_max_bus_rate;
		nv_card->get_agp_status = NULL;
		nv_card->get_agp_fw_status = NULL;
		nv_card->get_agp_sba_status = NULL;
		nv_card->get_agp_supported_rates = NULL;
	}
	else
	{
		nv_card->get_bus_rate = NULL;
		nv_card->get_agp_status = NULL;
		nv_card->get_agp_fw_status = NULL;
		nv_card->get_agp_sba_status = NULL;
		nv_card->get_agp_supported_rates = NULL;
	}

	nv_card->get_memory_size = get_memory_size;
	nv_card->get_memory_type = get_memory_type;
	nv_card->get_memory_width = get_memory_width;

	/* Debugging stuff */
	nv_card->get_debug_info = get_debug_info;
}
