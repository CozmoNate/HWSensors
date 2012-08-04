//
//  NouveauDefinitions.h
//  HWSensors
//
//  Created by Kozlek on 02.08.12.
//

/*
 * Copyright 2007-2008 Nouveau Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef HWSensors_NouveauDefinitions_h
#define HWSensors_NouveauDefinitions_h

#define NV04_PFB_BOOT_0						0x00100000
#	define NV04_PFB_BOOT_0_RAM_AMOUNT			0x00000003
#	define NV04_PFB_BOOT_0_RAM_AMOUNT_32MB			0x00000000
#	define NV04_PFB_BOOT_0_RAM_AMOUNT_4MB			0x00000001
#	define NV04_PFB_BOOT_0_RAM_AMOUNT_8MB			0x00000002
#	define NV04_PFB_BOOT_0_RAM_AMOUNT_16MB			0x00000003
#	define NV04_PFB_BOOT_0_RAM_WIDTH_128			0x00000004
#	define NV04_PFB_BOOT_0_RAM_TYPE				0x00000028
#	define NV04_PFB_BOOT_0_RAM_TYPE_SGRAM_8MBIT		0x00000000
#	define NV04_PFB_BOOT_0_RAM_TYPE_SGRAM_16MBIT		0x00000008
#	define NV04_PFB_BOOT_0_RAM_TYPE_SGRAM_16MBIT_4BANK	0x00000010
#	define NV04_PFB_BOOT_0_RAM_TYPE_SDRAM_16MBIT		0x00000018
#	define NV04_PFB_BOOT_0_RAM_TYPE_SDRAM_64MBIT		0x00000020
#	define NV04_PFB_BOOT_0_RAM_TYPE_SDRAM_64MBITX16		0x00000028
#	define NV04_PFB_BOOT_0_UMA_ENABLE			0x00000100
#	define NV04_PFB_BOOT_0_UMA_SIZE				0x0000f000
#define NV04_PFB_DEBUG_0					0x00100080
#	define NV04_PFB_DEBUG_0_PAGE_MODE			0x00000001
#	define NV04_PFB_DEBUG_0_REFRESH_OFF			0x00000010
#	define NV04_PFB_DEBUG_0_REFRESH_COUNTX64		0x00003f00
#	define NV04_PFB_DEBUG_0_REFRESH_SLOW_CLK		0x00004000
#	define NV04_PFB_DEBUG_0_SAFE_MODE			0x00008000
#	define NV04_PFB_DEBUG_0_ALOM_ENABLE			0x00010000
#	define NV04_PFB_DEBUG_0_CASOE				0x00100000
#	define NV04_PFB_DEBUG_0_CKE_INVERT			0x10000000
#	define NV04_PFB_DEBUG_0_REFINC				0x20000000
#	define NV04_PFB_DEBUG_0_SAVE_POWER_OFF			0x40000000
#define NV04_PFB_CFG0						0x00100200
#	define NV04_PFB_CFG0_SCRAMBLE				0x20000000
#define NV04_PFB_CFG1						0x00100204
#define NV04_PFB_FIFO_DATA					0x0010020c
#	define NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK		0xfff00000
#	define NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_SHIFT		20
#define NV10_PFB_REFCTRL					0x00100210
#	define NV10_PFB_REFCTRL_VALID_1				(1 << 31)
#define NV04_PFB_PAD						0x0010021c
#	define NV04_PFB_PAD_CKE_NORMAL				(1 << 0)
#define NV10_PFB_TILE(i)                              (0x00100240 + (i*16))
#define NV10_PFB_TILE__SIZE					8
#define NV10_PFB_TLIMIT(i)                            (0x00100244 + (i*16))
#define NV10_PFB_TSIZE(i)                             (0x00100248 + (i*16))
#define NV10_PFB_TSTATUS(i)                           (0x0010024c + (i*16))
#define NV04_PFB_REF						0x001002d0
#	define NV04_PFB_REF_CMD_REFRESH				(1 << 0)
#define NV04_PFB_PRE						0x001002d4
#	define NV04_PFB_PRE_CMD_PRECHARGE			(1 << 0)
#define NV20_PFB_ZCOMP(i)                              (0x00100300 + 4*(i))
#	define NV20_PFB_ZCOMP_MODE_32				(4 << 24)
#	define NV20_PFB_ZCOMP_EN				(1 << 31)
#	define NV25_PFB_ZCOMP_MODE_16				(1 << 20)
#	define NV25_PFB_ZCOMP_MODE_32				(2 << 20)
#define NV10_PFB_CLOSE_PAGE2					0x0010033c
#define NV04_PFB_SCRAMBLE(i)                         (0x00100400 + 4 * (i))
#define NV40_PFB_TILE(i)                              (0x00100600 + (i*16))
#define NV40_PFB_TILE__SIZE_0					12
#define NV40_PFB_TILE__SIZE_1					15
#define NV40_PFB_TLIMIT(i)                            (0x00100604 + (i*16))
#define NV40_PFB_TSIZE(i)                             (0x00100608 + (i*16))
#define NV40_PFB_TSTATUS(i)                           (0x0010060c + (i*16))
#define NV40_PFB_UNK_800					0x00100800

#define NV_PEXTDEV_BOOT_0					0x00101000
#define NV_PEXTDEV_BOOT_0_RAMCFG				0x0000003c
#	define NV_PEXTDEV_BOOT_0_STRAP_FP_IFACE_12BIT		(8 << 12)
#define NV_PEXTDEV_BOOT_3					0x0010100c

#define NV_RAMIN                                           0x00700000

#define NV_RAMHT_HANDLE_OFFSET                             0
#define NV_RAMHT_CONTEXT_OFFSET                            4
#    define NV_RAMHT_CONTEXT_VALID                         (1<<31)
#    define NV_RAMHT_CONTEXT_CHANNEL_SHIFT                 24
#    define NV_RAMHT_CONTEXT_ENGINE_SHIFT                  16
#        define NV_RAMHT_CONTEXT_ENGINE_SOFTWARE           0
#        define NV_RAMHT_CONTEXT_ENGINE_GRAPHICS           1
#    define NV_RAMHT_CONTEXT_INSTANCE_SHIFT                0
#    define NV40_RAMHT_CONTEXT_CHANNEL_SHIFT               23
#    define NV40_RAMHT_CONTEXT_ENGINE_SHIFT                20
#    define NV40_RAMHT_CONTEXT_INSTANCE_SHIFT              0

/* Some object classes we care about in the drm */
#define NV_CLASS_DMA_FROM_MEMORY                           0x00000002
#define NV_CLASS_DMA_TO_MEMORY                             0x00000003
#define NV_CLASS_NULL                                      0x00000030
#define NV_CLASS_DMA_IN_MEMORY                             0x0000003D

#define NV03_USER(i)                             (0x00800000+(i*NV03_USER_SIZE))
#define NV03_USER__SIZE                                                       16
#define NV10_USER__SIZE                                                       32
#define NV03_USER_SIZE                                                0x00010000
#define NV03_USER_DMA_PUT(i)                     (0x00800040+(i*NV03_USER_SIZE))
#define NV03_USER_DMA_PUT__SIZE                                               16
#define NV10_USER_DMA_PUT__SIZE                                               32
#define NV03_USER_DMA_GET(i)                     (0x00800044+(i*NV03_USER_SIZE))
#define NV03_USER_DMA_GET__SIZE                                               16
#define NV10_USER_DMA_GET__SIZE                                               32
#define NV03_USER_REF_CNT(i)                     (0x00800048+(i*NV03_USER_SIZE))
#define NV03_USER_REF_CNT__SIZE                                               16
#define NV10_USER_REF_CNT__SIZE                                               32

#define NV40_USER(i)                             (0x00c00000+(i*NV40_USER_SIZE))
#define NV40_USER_SIZE                                                0x00001000
#define NV40_USER_DMA_PUT(i)                     (0x00c00040+(i*NV40_USER_SIZE))
#define NV40_USER_DMA_PUT__SIZE                                               32
#define NV40_USER_DMA_GET(i)                     (0x00c00044+(i*NV40_USER_SIZE))
#define NV40_USER_DMA_GET__SIZE                                               32
#define NV40_USER_REF_CNT(i)                     (0x00c00048+(i*NV40_USER_SIZE))
#define NV40_USER_REF_CNT__SIZE                                               32

#define NV50_USER(i)                             (0x00c00000+(i*NV50_USER_SIZE))
#define NV50_USER_SIZE                                                0x00002000
#define NV50_USER_DMA_PUT(i)                     (0x00c00040+(i*NV50_USER_SIZE))
#define NV50_USER_DMA_PUT__SIZE                                              128
#define NV50_USER_DMA_GET(i)                     (0x00c00044+(i*NV50_USER_SIZE))
#define NV50_USER_DMA_GET__SIZE                                              128
#define NV50_USER_REF_CNT(i)                     (0x00c00048+(i*NV50_USER_SIZE))
#define NV50_USER_REF_CNT__SIZE                                              128

#define NV03_FIFO_SIZE                                     0x8000UL

#define NV03_PMC_BOOT_0                                    0x00000000
#define NV03_PMC_BOOT_1                                    0x00000004
#define NV03_PMC_INTR_0                                    0x00000100
#    define NV_PMC_INTR_0_PFIFO_PENDING                        (1<<8)
#    define NV_PMC_INTR_0_PGRAPH_PENDING                      (1<<12)
#    define NV_PMC_INTR_0_NV50_I2C_PENDING                    (1<<21)
#    define NV_PMC_INTR_0_CRTC0_PENDING                       (1<<24)
#    define NV_PMC_INTR_0_CRTC1_PENDING                       (1<<25)
#    define NV_PMC_INTR_0_NV50_DISPLAY_PENDING                (1<<26)
#    define NV_PMC_INTR_0_CRTCn_PENDING                       (3<<24)
#define NV03_PMC_INTR_EN_0                                 0x00000140
#    define NV_PMC_INTR_EN_0_MASTER_ENABLE                     (1<<0)
#define NV03_PMC_ENABLE                                    0x00000200
#    define NV_PMC_ENABLE_PFIFO                                (1<<8)
#    define NV_PMC_ENABLE_PGRAPH                              (1<<12)
/* Disabling the below bit breaks newer (G7X only?) mobile chipsets,
 * the card will hang early on in the X init process.
 */
#    define NV_PMC_ENABLE_UNK13                               (1<<13)
#define NV40_PMC_GRAPH_UNITS				   0x00001540
#define NV40_PMC_BACKLIGHT				   0x000015f0
#	define NV40_PMC_BACKLIGHT_MASK			   0x001f0000
#define NV40_PMC_1700                                      0x00001700
#define NV40_PMC_1704                                      0x00001704
#define NV40_PMC_1708                                      0x00001708
#define NV40_PMC_170C                                      0x0000170C

/* probably PMC ? */
#define NV50_PUNK_BAR0_PRAMIN                              0x00001700
#define NV50_PUNK_BAR_CFG_BASE                             0x00001704
#define NV50_PUNK_BAR_CFG_BASE_VALID                          (1<<30)
#define NV50_PUNK_BAR1_CTXDMA                              0x00001708
#define NV50_PUNK_BAR1_CTXDMA_VALID                           (1<<31)
#define NV50_PUNK_BAR3_CTXDMA                              0x0000170C
#define NV50_PUNK_BAR3_CTXDMA_VALID                           (1<<31)
#define NV50_PUNK_UNK1710                                  0x00001710

#define NV04_PBUS_PCI_NV_1                                 0x00001804
#define NV04_PBUS_PCI_NV_19                                0x0000184C
#define NV04_PBUS_PCI_NV_20				0x00001850
#	define NV04_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED		(0 << 0)
#	define NV04_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED		(1 << 0)

#define NV04_PTIMER_INTR_0                                 0x00009100
#define NV04_PTIMER_INTR_EN_0                              0x00009140
#define NV04_PTIMER_NUMERATOR                              0x00009200
#define NV04_PTIMER_DENOMINATOR                            0x00009210
#define NV04_PTIMER_TIME_0                                 0x00009400
#define NV04_PTIMER_TIME_1                                 0x00009410
#define NV04_PTIMER_ALARM_0                                0x00009420

#define NV04_PGRAPH_DEBUG_0                                0x00400080
#define NV04_PGRAPH_DEBUG_1                                0x00400084
#define NV04_PGRAPH_DEBUG_2                                0x00400088
#define NV04_PGRAPH_DEBUG_3                                0x0040008c
#define NV10_PGRAPH_DEBUG_4                                0x00400090
#define NV03_PGRAPH_INTR                                   0x00400100
#define NV03_PGRAPH_NSTATUS                                0x00400104
#    define NV04_PGRAPH_NSTATUS_STATE_IN_USE                  (1<<11)
#    define NV04_PGRAPH_NSTATUS_INVALID_STATE                 (1<<12)
#    define NV04_PGRAPH_NSTATUS_BAD_ARGUMENT                  (1<<13)
#    define NV04_PGRAPH_NSTATUS_PROTECTION_FAULT              (1<<14)
#    define NV10_PGRAPH_NSTATUS_STATE_IN_USE                  (1<<23)
#    define NV10_PGRAPH_NSTATUS_INVALID_STATE                 (1<<24)
#    define NV10_PGRAPH_NSTATUS_BAD_ARGUMENT                  (1<<25)
#    define NV10_PGRAPH_NSTATUS_PROTECTION_FAULT              (1<<26)
#define NV03_PGRAPH_NSOURCE                                0x00400108
#    define NV03_PGRAPH_NSOURCE_NOTIFICATION                   (1<<0)
#    define NV03_PGRAPH_NSOURCE_DATA_ERROR                     (1<<1)
#    define NV03_PGRAPH_NSOURCE_PROTECTION_ERROR               (1<<2)
#    define NV03_PGRAPH_NSOURCE_RANGE_EXCEPTION                (1<<3)
#    define NV03_PGRAPH_NSOURCE_LIMIT_COLOR                    (1<<4)
#    define NV03_PGRAPH_NSOURCE_LIMIT_ZETA                     (1<<5)
#    define NV03_PGRAPH_NSOURCE_ILLEGAL_MTHD                   (1<<6)
#    define NV03_PGRAPH_NSOURCE_DMA_R_PROTECTION               (1<<7)
#    define NV03_PGRAPH_NSOURCE_DMA_W_PROTECTION               (1<<8)
#    define NV03_PGRAPH_NSOURCE_FORMAT_EXCEPTION               (1<<9)
#    define NV03_PGRAPH_NSOURCE_PATCH_EXCEPTION               (1<<10)
#    define NV03_PGRAPH_NSOURCE_STATE_INVALID                 (1<<11)
#    define NV03_PGRAPH_NSOURCE_DOUBLE_NOTIFY                 (1<<12)
#    define NV03_PGRAPH_NSOURCE_NOTIFY_IN_USE                 (1<<13)
#    define NV03_PGRAPH_NSOURCE_METHOD_CNT                    (1<<14)
#    define NV03_PGRAPH_NSOURCE_BFR_NOTIFICATION              (1<<15)
#    define NV03_PGRAPH_NSOURCE_DMA_VTX_PROTECTION            (1<<16)
#    define NV03_PGRAPH_NSOURCE_DMA_WIDTH_A                   (1<<17)
#    define NV03_PGRAPH_NSOURCE_DMA_WIDTH_B                   (1<<18)
#define NV03_PGRAPH_INTR_EN                                0x00400140
#define NV40_PGRAPH_INTR_EN                                0x0040013C
#    define NV_PGRAPH_INTR_NOTIFY                              (1<<0)
#    define NV_PGRAPH_INTR_MISSING_HW                          (1<<4)
#    define NV_PGRAPH_INTR_CONTEXT_SWITCH                     (1<<12)
#    define NV_PGRAPH_INTR_BUFFER_NOTIFY                      (1<<16)
#    define NV_PGRAPH_INTR_ERROR                              (1<<20)
#define NV10_PGRAPH_CTX_CONTROL                            0x00400144
#define NV10_PGRAPH_CTX_USER                               0x00400148
#define NV10_PGRAPH_CTX_SWITCH(i)                         (0x0040014C + 0x4*(i))
#define NV04_PGRAPH_CTX_SWITCH1                            0x00400160
#define NV10_PGRAPH_CTX_CACHE(i, j)                       (0x00400160	\
+ 0x4*(i) + 0x20*(j))
#define NV04_PGRAPH_CTX_SWITCH2                            0x00400164
#define NV04_PGRAPH_CTX_SWITCH3                            0x00400168
#define NV04_PGRAPH_CTX_SWITCH4                            0x0040016C
#define NV04_PGRAPH_CTX_CONTROL                            0x00400170
#define NV04_PGRAPH_CTX_USER                               0x00400174
#define NV04_PGRAPH_CTX_CACHE1                             0x00400180
#define NV03_PGRAPH_CTX_CONTROL                            0x00400190
#define NV03_PGRAPH_CTX_USER                               0x00400194
#define NV04_PGRAPH_CTX_CACHE2                             0x004001A0
#define NV04_PGRAPH_CTX_CACHE3                             0x004001C0
#define NV04_PGRAPH_CTX_CACHE4                             0x004001E0
#define NV40_PGRAPH_CTXCTL_0304                            0x00400304
#define NV40_PGRAPH_CTXCTL_0304_XFER_CTX                   0x00000001
#define NV40_PGRAPH_CTXCTL_UCODE_STAT                      0x00400308
#define NV40_PGRAPH_CTXCTL_UCODE_STAT_IP_MASK              0xff000000
#define NV40_PGRAPH_CTXCTL_UCODE_STAT_IP_SHIFT                     24
#define NV40_PGRAPH_CTXCTL_UCODE_STAT_OP_MASK              0x00ffffff
#define NV40_PGRAPH_CTXCTL_0310                            0x00400310
#define NV40_PGRAPH_CTXCTL_0310_XFER_SAVE                  0x00000020
#define NV40_PGRAPH_CTXCTL_0310_XFER_LOAD                  0x00000040
#define NV40_PGRAPH_CTXCTL_030C                            0x0040030c
#define NV40_PGRAPH_CTXCTL_UCODE_INDEX                     0x00400324
#define NV40_PGRAPH_CTXCTL_UCODE_DATA                      0x00400328
#define NV40_PGRAPH_CTXCTL_CUR                             0x0040032c
#define NV40_PGRAPH_CTXCTL_CUR_LOADED                      0x01000000
#define NV40_PGRAPH_CTXCTL_CUR_INSTANCE                    0x000FFFFF
#define NV40_PGRAPH_CTXCTL_NEXT                            0x00400330
#define NV40_PGRAPH_CTXCTL_NEXT_INSTANCE                   0x000fffff
#define NV50_PGRAPH_CTXCTL_CUR                             0x0040032c
#define NV50_PGRAPH_CTXCTL_CUR_LOADED                      0x80000000
#define NV50_PGRAPH_CTXCTL_CUR_INSTANCE                    0x00ffffff
#define NV50_PGRAPH_CTXCTL_NEXT                            0x00400330
#define NV50_PGRAPH_CTXCTL_NEXT_INSTANCE                   0x00ffffff
#define NV03_PGRAPH_ABS_X_RAM                              0x00400400
#define NV03_PGRAPH_ABS_Y_RAM                              0x00400480
#define NV03_PGRAPH_X_MISC                                 0x00400500
#define NV03_PGRAPH_Y_MISC                                 0x00400504
#define NV04_PGRAPH_VALID1                                 0x00400508
#define NV04_PGRAPH_SOURCE_COLOR                           0x0040050C
#define NV04_PGRAPH_MISC24_0                               0x00400510
#define NV03_PGRAPH_XY_LOGIC_MISC0                         0x00400514
#define NV03_PGRAPH_XY_LOGIC_MISC1                         0x00400518
#define NV03_PGRAPH_XY_LOGIC_MISC2                         0x0040051C
#define NV03_PGRAPH_XY_LOGIC_MISC3                         0x00400520
#define NV03_PGRAPH_CLIPX_0                                0x00400524
#define NV03_PGRAPH_CLIPX_1                                0x00400528
#define NV03_PGRAPH_CLIPY_0                                0x0040052C
#define NV03_PGRAPH_CLIPY_1                                0x00400530
#define NV03_PGRAPH_ABS_ICLIP_XMAX                         0x00400534
#define NV03_PGRAPH_ABS_ICLIP_YMAX                         0x00400538
#define NV03_PGRAPH_ABS_UCLIP_XMIN                         0x0040053C
#define NV03_PGRAPH_ABS_UCLIP_YMIN                         0x00400540
#define NV03_PGRAPH_ABS_UCLIP_XMAX                         0x00400544
#define NV03_PGRAPH_ABS_UCLIP_YMAX                         0x00400548
#define NV03_PGRAPH_ABS_UCLIPA_XMIN                        0x00400560
#define NV03_PGRAPH_ABS_UCLIPA_YMIN                        0x00400564
#define NV03_PGRAPH_ABS_UCLIPA_XMAX                        0x00400568
#define NV03_PGRAPH_ABS_UCLIPA_YMAX                        0x0040056C
#define NV04_PGRAPH_MISC24_1                               0x00400570
#define NV04_PGRAPH_MISC24_2                               0x00400574
#define NV04_PGRAPH_VALID2                                 0x00400578
#define NV04_PGRAPH_PASSTHRU_0                             0x0040057C
#define NV04_PGRAPH_PASSTHRU_1                             0x00400580
#define NV04_PGRAPH_PASSTHRU_2                             0x00400584
#define NV10_PGRAPH_DIMX_TEXTURE                           0x00400588
#define NV10_PGRAPH_WDIMX_TEXTURE                          0x0040058C
#define NV04_PGRAPH_COMBINE_0_ALPHA                        0x00400590
#define NV04_PGRAPH_COMBINE_0_COLOR                        0x00400594
#define NV04_PGRAPH_COMBINE_1_ALPHA                        0x00400598
#define NV04_PGRAPH_COMBINE_1_COLOR                        0x0040059C
#define NV04_PGRAPH_FORMAT_0                               0x004005A8
#define NV04_PGRAPH_FORMAT_1                               0x004005AC
#define NV04_PGRAPH_FILTER_0                               0x004005B0
#define NV04_PGRAPH_FILTER_1                               0x004005B4
#define NV03_PGRAPH_MONO_COLOR0                            0x00400600
#define NV04_PGRAPH_ROP3                                   0x00400604
#define NV04_PGRAPH_BETA_AND                               0x00400608
#define NV04_PGRAPH_BETA_PREMULT                           0x0040060C
#define NV04_PGRAPH_LIMIT_VIOL_PIX                         0x00400610
#define NV04_PGRAPH_FORMATS                                0x00400618
#define NV10_PGRAPH_DEBUG_2                                0x00400620
#define NV04_PGRAPH_BOFFSET0                               0x00400640
#define NV04_PGRAPH_BOFFSET1                               0x00400644
#define NV04_PGRAPH_BOFFSET2                               0x00400648
#define NV04_PGRAPH_BOFFSET3                               0x0040064C
#define NV04_PGRAPH_BOFFSET4                               0x00400650
#define NV04_PGRAPH_BOFFSET5                               0x00400654
#define NV04_PGRAPH_BBASE0                                 0x00400658
#define NV04_PGRAPH_BBASE1                                 0x0040065C
#define NV04_PGRAPH_BBASE2                                 0x00400660
#define NV04_PGRAPH_BBASE3                                 0x00400664
#define NV04_PGRAPH_BBASE4                                 0x00400668
#define NV04_PGRAPH_BBASE5                                 0x0040066C
#define NV04_PGRAPH_BPITCH0                                0x00400670
#define NV04_PGRAPH_BPITCH1                                0x00400674
#define NV04_PGRAPH_BPITCH2                                0x00400678
#define NV04_PGRAPH_BPITCH3                                0x0040067C
#define NV04_PGRAPH_BPITCH4                                0x00400680
#define NV04_PGRAPH_BLIMIT0                                0x00400684
#define NV04_PGRAPH_BLIMIT1                                0x00400688
#define NV04_PGRAPH_BLIMIT2                                0x0040068C
#define NV04_PGRAPH_BLIMIT3                                0x00400690
#define NV04_PGRAPH_BLIMIT4                                0x00400694
#define NV04_PGRAPH_BLIMIT5                                0x00400698
#define NV04_PGRAPH_BSWIZZLE2                              0x0040069C
#define NV04_PGRAPH_BSWIZZLE5                              0x004006A0
#define NV03_PGRAPH_STATUS                                 0x004006B0
#define NV04_PGRAPH_STATUS                                 0x00400700
#    define NV40_PGRAPH_STATUS_SYNC_STALL                  0x00004000
#define NV04_PGRAPH_TRAPPED_ADDR                           0x00400704
#define NV04_PGRAPH_TRAPPED_DATA                           0x00400708
#define NV04_PGRAPH_SURFACE                                0x0040070C
#define NV10_PGRAPH_TRAPPED_DATA_HIGH                      0x0040070C
#define NV04_PGRAPH_STATE                                  0x00400710
#define NV10_PGRAPH_SURFACE                                0x00400710
#define NV04_PGRAPH_NOTIFY                                 0x00400714
#define NV10_PGRAPH_STATE                                  0x00400714
#define NV10_PGRAPH_NOTIFY                                 0x00400718

#define NV04_PGRAPH_FIFO                                   0x00400720

#define NV04_PGRAPH_BPIXEL                                 0x00400724
#define NV10_PGRAPH_RDI_INDEX                              0x00400750
#define NV04_PGRAPH_FFINTFC_ST2                            0x00400754
#define NV10_PGRAPH_RDI_DATA                               0x00400754
#define NV04_PGRAPH_DMA_PITCH                              0x00400760
#define NV10_PGRAPH_FFINTFC_FIFO_PTR                       0x00400760
#define NV04_PGRAPH_DVD_COLORFMT                           0x00400764
#define NV10_PGRAPH_FFINTFC_ST2                            0x00400764
#define NV04_PGRAPH_SCALED_FORMAT                          0x00400768
#define NV10_PGRAPH_FFINTFC_ST2_DL                         0x00400768
#define NV10_PGRAPH_FFINTFC_ST2_DH                         0x0040076c
#define NV10_PGRAPH_DMA_PITCH                              0x00400770
#define NV10_PGRAPH_DVD_COLORFMT                           0x00400774
#define NV10_PGRAPH_SCALED_FORMAT                          0x00400778
#define NV20_PGRAPH_CHANNEL_CTX_TABLE                      0x00400780
#define NV20_PGRAPH_CHANNEL_CTX_POINTER                    0x00400784
#define NV20_PGRAPH_CHANNEL_CTX_XFER                       0x00400788
#define NV20_PGRAPH_CHANNEL_CTX_XFER_LOAD                  0x00000001
#define NV20_PGRAPH_CHANNEL_CTX_XFER_SAVE                  0x00000002
#define NV04_PGRAPH_PATT_COLOR0                            0x00400800
#define NV04_PGRAPH_PATT_COLOR1                            0x00400804
#define NV04_PGRAPH_PATTERN                                0x00400808
#define NV04_PGRAPH_PATTERN_SHAPE                          0x00400810
#define NV04_PGRAPH_CHROMA                                 0x00400814
#define NV04_PGRAPH_CONTROL0                               0x00400818
#define NV04_PGRAPH_CONTROL1                               0x0040081C
#define NV04_PGRAPH_CONTROL2                               0x00400820
#define NV04_PGRAPH_BLEND                                  0x00400824
#define NV04_PGRAPH_STORED_FMT                             0x00400830
#define NV04_PGRAPH_PATT_COLORRAM                          0x00400900
#define NV20_PGRAPH_TILE(i)                                (0x00400900 + (i*16))
#define NV20_PGRAPH_TLIMIT(i)                              (0x00400904 + (i*16))
#define NV20_PGRAPH_TSIZE(i)                               (0x00400908 + (i*16))
#define NV20_PGRAPH_TSTATUS(i)                             (0x0040090C + (i*16))
#define NV20_PGRAPH_ZCOMP(i)                               (0x00400980 + 4*(i))
#define NV10_PGRAPH_TILE(i)                                (0x00400B00 + (i*16))
#define NV10_PGRAPH_TLIMIT(i)                              (0x00400B04 + (i*16))
#define NV10_PGRAPH_TSIZE(i)                               (0x00400B08 + (i*16))
#define NV10_PGRAPH_TSTATUS(i)                             (0x00400B0C + (i*16))
#define NV04_PGRAPH_U_RAM                                  0x00400D00
#define NV47_PGRAPH_TILE(i)                                (0x00400D00 + (i*16))
#define NV47_PGRAPH_TLIMIT(i)                              (0x00400D04 + (i*16))
#define NV47_PGRAPH_TSIZE(i)                               (0x00400D08 + (i*16))
#define NV47_PGRAPH_TSTATUS(i)                             (0x00400D0C + (i*16))
#define NV04_PGRAPH_V_RAM                                  0x00400D40
#define NV04_PGRAPH_W_RAM                                  0x00400D80
#define NV10_PGRAPH_COMBINER0_IN_ALPHA                     0x00400E40
#define NV10_PGRAPH_COMBINER1_IN_ALPHA                     0x00400E44
#define NV10_PGRAPH_COMBINER0_IN_RGB                       0x00400E48
#define NV10_PGRAPH_COMBINER1_IN_RGB                       0x00400E4C
#define NV10_PGRAPH_COMBINER_COLOR0                        0x00400E50
#define NV10_PGRAPH_COMBINER_COLOR1                        0x00400E54
#define NV10_PGRAPH_COMBINER0_OUT_ALPHA                    0x00400E58
#define NV10_PGRAPH_COMBINER1_OUT_ALPHA                    0x00400E5C
#define NV10_PGRAPH_COMBINER0_OUT_RGB                      0x00400E60
#define NV10_PGRAPH_COMBINER1_OUT_RGB                      0x00400E64
#define NV10_PGRAPH_COMBINER_FINAL0                        0x00400E68
#define NV10_PGRAPH_COMBINER_FINAL1                        0x00400E6C
#define NV10_PGRAPH_WINDOWCLIP_HORIZONTAL                  0x00400F00
#define NV10_PGRAPH_WINDOWCLIP_VERTICAL                    0x00400F20
#define NV10_PGRAPH_XFMODE0                                0x00400F40
#define NV10_PGRAPH_XFMODE1                                0x00400F44
#define NV10_PGRAPH_GLOBALSTATE0                           0x00400F48
#define NV10_PGRAPH_GLOBALSTATE1                           0x00400F4C
#define NV10_PGRAPH_PIPE_ADDRESS                           0x00400F50
#define NV10_PGRAPH_PIPE_DATA                              0x00400F54
#define NV04_PGRAPH_DMA_START_0                            0x00401000
#define NV04_PGRAPH_DMA_START_1                            0x00401004
#define NV04_PGRAPH_DMA_LENGTH                             0x00401008
#define NV04_PGRAPH_DMA_MISC                               0x0040100C
#define NV04_PGRAPH_DMA_DATA_0                             0x00401020
#define NV04_PGRAPH_DMA_DATA_1                             0x00401024
#define NV04_PGRAPH_DMA_RM                                 0x00401030
#define NV04_PGRAPH_DMA_A_XLATE_INST                       0x00401040
#define NV04_PGRAPH_DMA_A_CONTROL                          0x00401044
#define NV04_PGRAPH_DMA_A_LIMIT                            0x00401048
#define NV04_PGRAPH_DMA_A_TLB_PTE                          0x0040104C
#define NV04_PGRAPH_DMA_A_TLB_TAG                          0x00401050
#define NV04_PGRAPH_DMA_A_ADJ_OFFSET                       0x00401054
#define NV04_PGRAPH_DMA_A_OFFSET                           0x00401058
#define NV04_PGRAPH_DMA_A_SIZE                             0x0040105C
#define NV04_PGRAPH_DMA_A_Y_SIZE                           0x00401060
#define NV04_PGRAPH_DMA_B_XLATE_INST                       0x00401080
#define NV04_PGRAPH_DMA_B_CONTROL                          0x00401084
#define NV04_PGRAPH_DMA_B_LIMIT                            0x00401088
#define NV04_PGRAPH_DMA_B_TLB_PTE                          0x0040108C
#define NV04_PGRAPH_DMA_B_TLB_TAG                          0x00401090
#define NV04_PGRAPH_DMA_B_ADJ_OFFSET                       0x00401094
#define NV04_PGRAPH_DMA_B_OFFSET                           0x00401098
#define NV04_PGRAPH_DMA_B_SIZE                             0x0040109C
#define NV04_PGRAPH_DMA_B_Y_SIZE                           0x004010A0
#define NV40_PGRAPH_TILE1(i)                               (0x00406900 + (i*16))
#define NV40_PGRAPH_TLIMIT1(i)                             (0x00406904 + (i*16))
#define NV40_PGRAPH_TSIZE1(i)                              (0x00406908 + (i*16))
#define NV40_PGRAPH_TSTATUS1(i)                            (0x0040690C + (i*16))


/* It's a guess that this works on NV03. Confirmed on NV04, though */
#define NV04_PFIFO_DELAY_0                                 0x00002040
#define NV04_PFIFO_DMA_TIMESLICE                           0x00002044
#define NV04_PFIFO_NEXT_CHANNEL                            0x00002050
#define NV03_PFIFO_INTR_0                                  0x00002100
#define NV03_PFIFO_INTR_EN_0                               0x00002140
#    define NV_PFIFO_INTR_CACHE_ERROR                          (1<<0)
#    define NV_PFIFO_INTR_RUNOUT                               (1<<4)
#    define NV_PFIFO_INTR_RUNOUT_OVERFLOW                      (1<<8)
#    define NV_PFIFO_INTR_DMA_PUSHER                          (1<<12)
#    define NV_PFIFO_INTR_DMA_PT                              (1<<16)
#    define NV_PFIFO_INTR_SEMAPHORE                           (1<<20)
#    define NV_PFIFO_INTR_ACQUIRE_TIMEOUT                     (1<<24)
#define NV03_PFIFO_RAMHT                                   0x00002210
#define NV03_PFIFO_RAMFC                                   0x00002214
#define NV03_PFIFO_RAMRO                                   0x00002218
#define NV40_PFIFO_RAMFC                                   0x00002220
#define NV03_PFIFO_CACHES                                  0x00002500
#define NV04_PFIFO_MODE                                    0x00002504
#define NV04_PFIFO_DMA                                     0x00002508
#define NV04_PFIFO_SIZE                                    0x0000250c
#define NV50_PFIFO_CTX_TABLE(c)                        (0x2600+(c)*4)
#define NV50_PFIFO_CTX_TABLE__SIZE                                128
#define NV50_PFIFO_CTX_TABLE_CHANNEL_ENABLED                  (1<<31)
#define NV50_PFIFO_CTX_TABLE_UNK30_BAD                        (1<<30)
#define NV50_PFIFO_CTX_TABLE_INSTANCE_MASK_G80             0x0FFFFFFF
#define NV50_PFIFO_CTX_TABLE_INSTANCE_MASK_G84             0x00FFFFFF
#define NV03_PFIFO_CACHE0_PUSH0                            0x00003000
#define NV03_PFIFO_CACHE0_PULL0                            0x00003040
#define NV04_PFIFO_CACHE0_PULL0                            0x00003050
#define NV04_PFIFO_CACHE0_PULL1                            0x00003054
#define NV03_PFIFO_CACHE1_PUSH0                            0x00003200
#define NV03_PFIFO_CACHE1_PUSH1                            0x00003204
#define NV03_PFIFO_CACHE1_PUSH1_DMA                            (1<<8)
#define NV40_PFIFO_CACHE1_PUSH1_DMA                           (1<<16)
#define NV03_PFIFO_CACHE1_PUSH1_CHID_MASK                  0x0000000f
#define NV10_PFIFO_CACHE1_PUSH1_CHID_MASK                  0x0000001f
#define NV50_PFIFO_CACHE1_PUSH1_CHID_MASK                  0x0000007f
#define NV03_PFIFO_CACHE1_PUT                              0x00003210
#define NV04_PFIFO_CACHE1_DMA_PUSH                         0x00003220
#define NV04_PFIFO_CACHE1_DMA_FETCH                        0x00003224
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_8_BYTES         0x00000000
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_16_BYTES        0x00000008
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_24_BYTES        0x00000010
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_32_BYTES        0x00000018
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_40_BYTES        0x00000020
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_48_BYTES        0x00000028
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_56_BYTES        0x00000030
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_64_BYTES        0x00000038
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_72_BYTES        0x00000040
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_80_BYTES        0x00000048
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_88_BYTES        0x00000050
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_96_BYTES        0x00000058
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_104_BYTES       0x00000060
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_112_BYTES       0x00000068
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_120_BYTES       0x00000070
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_128_BYTES       0x00000078
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_136_BYTES       0x00000080
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_144_BYTES       0x00000088
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_152_BYTES       0x00000090
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_160_BYTES       0x00000098
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_168_BYTES       0x000000A0
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_176_BYTES       0x000000A8
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_184_BYTES       0x000000B0
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_192_BYTES       0x000000B8
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_200_BYTES       0x000000C0
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_208_BYTES       0x000000C8
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_216_BYTES       0x000000D0
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_224_BYTES       0x000000D8
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_232_BYTES       0x000000E0
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_240_BYTES       0x000000E8
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_248_BYTES       0x000000F0
#    define NV_PFIFO_CACHE1_DMA_FETCH_TRIG_256_BYTES       0x000000F8
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE                 0x0000E000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_32_BYTES        0x00000000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_64_BYTES        0x00002000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_96_BYTES        0x00004000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_128_BYTES       0x00006000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_160_BYTES       0x00008000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_192_BYTES       0x0000A000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_224_BYTES       0x0000C000
#    define NV_PFIFO_CACHE1_DMA_FETCH_SIZE_256_BYTES       0x0000E000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS             0x001F0000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_0           0x00000000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_1           0x00010000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_2           0x00020000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_3           0x00030000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_4           0x00040000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_5           0x00050000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_6           0x00060000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_7           0x00070000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_8           0x00080000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_9           0x00090000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_10          0x000A0000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_11          0x000B0000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_12          0x000C0000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_13          0x000D0000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_14          0x000E0000
#    define NV_PFIFO_CACHE1_DMA_FETCH_MAX_REQS_15          0x000F0000
#    define NV_PFIFO_CACHE1_ENDIAN                         0x80000000
#    define NV_PFIFO_CACHE1_LITTLE_ENDIAN                  0x7FFFFFFF
#    define NV_PFIFO_CACHE1_BIG_ENDIAN                     0x80000000
#define NV04_PFIFO_CACHE1_DMA_STATE                        0x00003228
#define NV04_PFIFO_CACHE1_DMA_INSTANCE                     0x0000322c
#define NV04_PFIFO_CACHE1_DMA_CTL                          0x00003230
#define NV04_PFIFO_CACHE1_DMA_PUT                          0x00003240
#define NV04_PFIFO_CACHE1_DMA_GET                          0x00003244
#define NV10_PFIFO_CACHE1_REF_CNT                          0x00003248
#define NV10_PFIFO_CACHE1_DMA_SUBROUTINE                   0x0000324C
#define NV03_PFIFO_CACHE1_PULL0                            0x00003240
#define NV04_PFIFO_CACHE1_PULL0                            0x00003250
#    define NV04_PFIFO_CACHE1_PULL0_HASH_FAILED            0x00000010
#    define NV04_PFIFO_CACHE1_PULL0_HASH_BUSY              0x00001000
#define NV03_PFIFO_CACHE1_PULL1                            0x00003250
#define NV04_PFIFO_CACHE1_PULL1                            0x00003254
#define NV04_PFIFO_CACHE1_HASH                             0x00003258
#define NV10_PFIFO_CACHE1_ACQUIRE_TIMEOUT                  0x00003260
#define NV10_PFIFO_CACHE1_ACQUIRE_TIMESTAMP                0x00003264
#define NV10_PFIFO_CACHE1_ACQUIRE_VALUE                    0x00003268
#define NV10_PFIFO_CACHE1_SEMAPHORE                        0x0000326C
#define NV03_PFIFO_CACHE1_GET                              0x00003270
#define NV04_PFIFO_CACHE1_ENGINE                           0x00003280
#define NV04_PFIFO_CACHE1_DMA_DCOUNT                       0x000032A0
#define NV40_PFIFO_GRCTX_INSTANCE                          0x000032E0
#define NV40_PFIFO_UNK32E4                                 0x000032E4
#define NV04_PFIFO_CACHE1_METHOD(i)                (0x00003800+(i*8))
#define NV04_PFIFO_CACHE1_DATA(i)                  (0x00003804+(i*8))
#define NV40_PFIFO_CACHE1_METHOD(i)                (0x00090000+(i*8))
#define NV40_PFIFO_CACHE1_DATA(i)                  (0x00090004+(i*8))

#define NV_CRTC0_INTSTAT                                   0x00600100
#define NV_CRTC0_INTEN                                     0x00600140
#define NV_CRTC1_INTSTAT                                   0x00602100
#define NV_CRTC1_INTEN                                     0x00602140
#    define NV_CRTC_INTR_VBLANK                                (1<<0)

#define NV04_PRAMIN						0x00700000

/* Fifo commands. These are not regs, neither masks */
#define NV03_FIFO_CMD_JUMP                                 0x20000000
#define NV03_FIFO_CMD_JUMP_OFFSET_MASK                     0x1ffffffc
#define NV03_FIFO_CMD_REWIND                               (NV03_FIFO_CMD_JUMP | (0 & NV03_FIFO_CMD_JUMP_OFFSET_MASK))

/* This is a partial import from rules-ng, a few things may be duplicated.
 * Eventually we should completely import everything from rules-ng.
 * For the moment check rules-ng for docs.
 */

#define NV50_PMC                                            0x00000000
#define NV50_PMC__LEN                                              0x1
#define NV50_PMC__ESIZE                                         0x2000
#    define NV50_PMC_BOOT_0                                 0x00000000
#        define NV50_PMC_BOOT_0_REVISION                    0x000000ff
#        define NV50_PMC_BOOT_0_REVISION__SHIFT                      0
#        define NV50_PMC_BOOT_0_ARCH                        0x0ff00000
#        define NV50_PMC_BOOT_0_ARCH__SHIFT                         20
#    define NV50_PMC_INTR_0                                 0x00000100
#        define NV50_PMC_INTR_0_PFIFO                           (1<<8)
#        define NV50_PMC_INTR_0_PGRAPH                         (1<<12)
#        define NV50_PMC_INTR_0_PTIMER                         (1<<20)
#        define NV50_PMC_INTR_0_HOTPLUG                        (1<<21)
#        define NV50_PMC_INTR_0_DISPLAY                        (1<<26)
#    define NV50_PMC_INTR_EN_0                              0x00000140
#        define NV50_PMC_INTR_EN_0_MASTER                       (1<<0)
#            define NV50_PMC_INTR_EN_0_MASTER_DISABLED          (0<<0)
#            define NV50_PMC_INTR_EN_0_MASTER_ENABLED           (1<<0)
#    define NV50_PMC_ENABLE                                 0x00000200
#        define NV50_PMC_ENABLE_PFIFO                           (1<<8)
#        define NV50_PMC_ENABLE_PGRAPH                         (1<<12)

#define NV50_PCONNECTOR                                     0x0000e000
#define NV50_PCONNECTOR__LEN                                       0x1
#define NV50_PCONNECTOR__ESIZE                                  0x1000
#    define NV50_PCONNECTOR_HOTPLUG_INTR                    0x0000e050
#        define NV50_PCONNECTOR_HOTPLUG_INTR_PLUG_I2C0          (1<<0)
#        define NV50_PCONNECTOR_HOTPLUG_INTR_PLUG_I2C1          (1<<1)
#        define NV50_PCONNECTOR_HOTPLUG_INTR_PLUG_I2C2          (1<<2)
#        define NV50_PCONNECTOR_HOTPLUG_INTR_PLUG_I2C3          (1<<3)
#        define NV50_PCONNECTOR_HOTPLUG_INTR_UNPLUG_I2C0       (1<<16)
#        define NV50_PCONNECTOR_HOTPLUG_INTR_UNPLUG_I2C1       (1<<17)
#        define NV50_PCONNECTOR_HOTPLUG_INTR_UNPLUG_I2C2       (1<<18)
#        define NV50_PCONNECTOR_HOTPLUG_INTR_UNPLUG_I2C3       (1<<19)
#    define NV50_PCONNECTOR_HOTPLUG_CTRL                    0x0000e054
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_PLUG_I2C0          (1<<0)
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_PLUG_I2C1          (1<<1)
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_PLUG_I2C2          (1<<2)
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_PLUG_I2C3          (1<<3)
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_UNPLUG_I2C0       (1<<16)
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_UNPLUG_I2C1       (1<<17)
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_UNPLUG_I2C2       (1<<18)
#        define NV50_PCONNECTOR_HOTPLUG_CTRL_UNPLUG_I2C3       (1<<19)
#    define NV50_PCONNECTOR_HOTPLUG_STATE                   0x0000e104
#        define NV50_PCONNECTOR_HOTPLUG_STATE_PIN_CONNECTED_I2C0 (1<<2)
#        define NV50_PCONNECTOR_HOTPLUG_STATE_PIN_CONNECTED_I2C1 (1<<6)
#        define NV50_PCONNECTOR_HOTPLUG_STATE_PIN_CONNECTED_I2C2 (1<<10)
#        define NV50_PCONNECTOR_HOTPLUG_STATE_PIN_CONNECTED_I2C3 (1<<14)
#    define NV50_PCONNECTOR_I2C_PORT_0                      0x0000e138
#    define NV50_PCONNECTOR_I2C_PORT_1                      0x0000e150
#    define NV50_PCONNECTOR_I2C_PORT_2                      0x0000e168
#    define NV50_PCONNECTOR_I2C_PORT_3                      0x0000e180
#    define NV50_PCONNECTOR_I2C_PORT_4                      0x0000e240
#    define NV50_PCONNECTOR_I2C_PORT_5                      0x0000e258

#define NV50_AUXCH_DATA_OUT(i, n)            ((n) * 4 + (i) * 0x50 + 0x0000e4c0)
#define NV50_AUXCH_DATA_OUT__SIZE                                             4
#define NV50_AUXCH_DATA_IN(i, n)             ((n) * 4 + (i) * 0x50 + 0x0000e4d0)
#define NV50_AUXCH_DATA_IN__SIZE                                              4
#define NV50_AUXCH_ADDR(i)                             ((i) * 0x50 + 0x0000e4e0)
#define NV50_AUXCH_CTRL(i)                             ((i) * 0x50 + 0x0000e4e4)
#define NV50_AUXCH_CTRL_LINKSTAT                                     0x01000000
#define NV50_AUXCH_CTRL_LINKSTAT_NOT_READY                           0x00000000
#define NV50_AUXCH_CTRL_LINKSTAT_READY                               0x01000000
#define NV50_AUXCH_CTRL_LINKEN                                       0x00100000
#define NV50_AUXCH_CTRL_LINKEN_DISABLED                              0x00000000
#define NV50_AUXCH_CTRL_LINKEN_ENABLED                               0x00100000
#define NV50_AUXCH_CTRL_EXEC                                         0x00010000
#define NV50_AUXCH_CTRL_EXEC_COMPLETE                                0x00000000
#define NV50_AUXCH_CTRL_EXEC_IN_PROCESS                              0x00010000
#define NV50_AUXCH_CTRL_CMD                                          0x0000f000
#define NV50_AUXCH_CTRL_CMD_SHIFT                                            12
#define NV50_AUXCH_CTRL_LEN                                          0x0000000f
#define NV50_AUXCH_CTRL_LEN_SHIFT                                             0
#define NV50_AUXCH_STAT(i)                             ((i) * 0x50 + 0x0000e4e8)
#define NV50_AUXCH_STAT_STATE                                        0x10000000
#define NV50_AUXCH_STAT_STATE_NOT_READY                              0x00000000
#define NV50_AUXCH_STAT_STATE_READY                                  0x10000000
#define NV50_AUXCH_STAT_REPLY                                        0x000f0000
#define NV50_AUXCH_STAT_REPLY_AUX                                    0x00030000
#define NV50_AUXCH_STAT_REPLY_AUX_ACK                                0x00000000
#define NV50_AUXCH_STAT_REPLY_AUX_NACK                               0x00010000
#define NV50_AUXCH_STAT_REPLY_AUX_DEFER                              0x00020000
#define NV50_AUXCH_STAT_REPLY_I2C                                    0x000c0000
#define NV50_AUXCH_STAT_REPLY_I2C_ACK                                0x00000000
#define NV50_AUXCH_STAT_REPLY_I2C_NACK                               0x00040000
#define NV50_AUXCH_STAT_REPLY_I2C_DEFER                              0x00080000
#define NV50_AUXCH_STAT_COUNT                                        0x0000001f

#define NV50_PBUS                                           0x00088000
#define NV50_PBUS__LEN                                             0x1
#define NV50_PBUS__ESIZE                                        0x1000
#    define NV50_PBUS_PCI_ID                                0x00088000
#        define NV50_PBUS_PCI_ID_VENDOR_ID                  0x0000ffff
#        define NV50_PBUS_PCI_ID_VENDOR_ID__SHIFT                    0
#        define NV50_PBUS_PCI_ID_DEVICE_ID                  0xffff0000
#        define NV50_PBUS_PCI_ID_DEVICE_ID__SHIFT                   16

#define NV50_PFB                                            0x00100000
#define NV50_PFB__LEN                                              0x1
#define NV50_PFB__ESIZE                                         0x1000

#define NV50_PEXTDEV                                        0x00101000
#define NV50_PEXTDEV__LEN                                          0x1
#define NV50_PEXTDEV__ESIZE                                     0x1000

#define NV50_PROM                                           0x00300000
#define NV50_PROM__LEN                                             0x1
#define NV50_PROM__ESIZE                                       0x10000

#define NV50_PGRAPH                                         0x00400000
#define NV50_PGRAPH__LEN                                           0x1
#define NV50_PGRAPH__ESIZE                                     0x10000

#define NV50_PDISPLAY                                                0x00610000
#define NV50_PDISPLAY_OBJECTS                                        0x00610010
#define NV50_PDISPLAY_INTR_0                                         0x00610020
#define NV50_PDISPLAY_INTR_1                                         0x00610024
#define NV50_PDISPLAY_INTR_1_VBLANK_CRTC                             0x0000000c
#define NV50_PDISPLAY_INTR_1_VBLANK_CRTC_SHIFT                                2
#define NV50_PDISPLAY_INTR_1_VBLANK_CRTC_(n)                   (1 << ((n) + 2))
#define NV50_PDISPLAY_INTR_1_VBLANK_CRTC_0                           0x00000004
#define NV50_PDISPLAY_INTR_1_VBLANK_CRTC_1                           0x00000008
#define NV50_PDISPLAY_INTR_1_CLK_UNK10                               0x00000010
#define NV50_PDISPLAY_INTR_1_CLK_UNK20                               0x00000020
#define NV50_PDISPLAY_INTR_1_CLK_UNK40                               0x00000040
#define NV50_PDISPLAY_INTR_EN_0                                      0x00610028
#define NV50_PDISPLAY_INTR_EN_1                                      0x0061002c
#define NV50_PDISPLAY_INTR_EN_1_VBLANK_CRTC                          0x0000000c
#define NV50_PDISPLAY_INTR_EN_1_VBLANK_CRTC_(n)                 (1 << ((n) + 2))
#define NV50_PDISPLAY_INTR_EN_1_VBLANK_CRTC_0                        0x00000004
#define NV50_PDISPLAY_INTR_EN_1_VBLANK_CRTC_1                        0x00000008
#define NV50_PDISPLAY_INTR_EN_1_CLK_UNK10                            0x00000010
#define NV50_PDISPLAY_INTR_EN_1_CLK_UNK20                            0x00000020
#define NV50_PDISPLAY_INTR_EN_1_CLK_UNK40                            0x00000040
#define NV50_PDISPLAY_UNK30_CTRL                                     0x00610030
#define NV50_PDISPLAY_UNK30_CTRL_UPDATE_VCLK0                        0x00000200
#define NV50_PDISPLAY_UNK30_CTRL_UPDATE_VCLK1                        0x00000400
#define NV50_PDISPLAY_UNK30_CTRL_PENDING                             0x80000000
#define NV50_PDISPLAY_TRAPPED_ADDR(i)                  ((i) * 0x08 + 0x00610080)
#define NV50_PDISPLAY_TRAPPED_DATA(i)                  ((i) * 0x08 + 0x00610084)
#define NV50_PDISPLAY_EVO_CTRL(i)                      ((i) * 0x10 + 0x00610200)
#define NV50_PDISPLAY_EVO_CTRL_DMA                                   0x00000010
#define NV50_PDISPLAY_EVO_CTRL_DMA_DISABLED                          0x00000000
#define NV50_PDISPLAY_EVO_CTRL_DMA_ENABLED                           0x00000010
#define NV50_PDISPLAY_EVO_DMA_CB(i)                    ((i) * 0x10 + 0x00610204)
#define NV50_PDISPLAY_EVO_DMA_CB_LOCATION                            0x00000002
#define NV50_PDISPLAY_EVO_DMA_CB_LOCATION_VRAM                       0x00000000
#define NV50_PDISPLAY_EVO_DMA_CB_LOCATION_SYSTEM                     0x00000002
#define NV50_PDISPLAY_EVO_DMA_CB_VALID                               0x00000001
#define NV50_PDISPLAY_EVO_UNK2(i)                      ((i) * 0x10 + 0x00610208)
#define NV50_PDISPLAY_EVO_HASH_TAG(i)                  ((i) * 0x10 + 0x0061020c)

#define NV50_PDISPLAY_CURSOR                                         0x00610270
#define NV50_PDISPLAY_CURSOR_CURSOR_CTRL2(i)           ((i) * 0x10 + 0x00610270)
#define NV50_PDISPLAY_CURSOR_CURSOR_CTRL2_ON                         0x00000001
#define NV50_PDISPLAY_CURSOR_CURSOR_CTRL2_STATUS                     0x00030000
#define NV50_PDISPLAY_CURSOR_CURSOR_CTRL2_STATUS_ACTIVE              0x00010000

#define NV50_PDISPLAY_PIO_CTRL                                       0x00610300
#define NV50_PDISPLAY_PIO_CTRL_PENDING                               0x80000000
#define NV50_PDISPLAY_PIO_CTRL_MTHD                                  0x00001ffc
#define NV50_PDISPLAY_PIO_CTRL_ENABLED                               0x00000001
#define NV50_PDISPLAY_PIO_DATA                                       0x00610304

#define NV50_PDISPLAY_CRTC_P(i, r)        ((i) * 0x540 + NV50_PDISPLAY_CRTC_##r)
#define NV50_PDISPLAY_CRTC_C(i, r)    (4 + (i) * 0x540 + NV50_PDISPLAY_CRTC_##r)
#define NV50_PDISPLAY_CRTC_UNK_0A18 /* mthd 0x0900 */                0x00610a18
#define NV50_PDISPLAY_CRTC_CLUT_MODE                                 0x00610a24
#define NV50_PDISPLAY_CRTC_INTERLACE                                 0x00610a48
#define NV50_PDISPLAY_CRTC_SCALE_CTRL                                0x00610a50
#define NV50_PDISPLAY_CRTC_CURSOR_CTRL                               0x00610a58
#define NV50_PDISPLAY_CRTC_UNK0A78 /* mthd 0x0904 */                 0x00610a78
#define NV50_PDISPLAY_CRTC_UNK0AB8                                   0x00610ab8
#define NV50_PDISPLAY_CRTC_DEPTH                                     0x00610ac8
#define NV50_PDISPLAY_CRTC_CLOCK                                     0x00610ad0
#define NV50_PDISPLAY_CRTC_COLOR_CTRL                                0x00610ae0
#define NV50_PDISPLAY_CRTC_SYNC_START_TO_BLANK_END                   0x00610ae8
#define NV50_PDISPLAY_CRTC_MODE_UNK1                                 0x00610af0
#define NV50_PDISPLAY_CRTC_DISPLAY_TOTAL                             0x00610af8
#define NV50_PDISPLAY_CRTC_SYNC_DURATION                             0x00610b00
#define NV50_PDISPLAY_CRTC_MODE_UNK2                                 0x00610b08
#define NV50_PDISPLAY_CRTC_UNK_0B10 /* mthd 0x0828 */                0x00610b10
#define NV50_PDISPLAY_CRTC_FB_SIZE                                   0x00610b18
#define NV50_PDISPLAY_CRTC_FB_PITCH                                  0x00610b20
#define NV50_PDISPLAY_CRTC_FB_PITCH_LINEAR                           0x00100000
#define NV50_PDISPLAY_CRTC_FB_POS                                    0x00610b28
#define NV50_PDISPLAY_CRTC_SCALE_CENTER_OFFSET                       0x00610b38
#define NV50_PDISPLAY_CRTC_REAL_RES                                  0x00610b40
#define NV50_PDISPLAY_CRTC_SCALE_RES1                                0x00610b48
#define NV50_PDISPLAY_CRTC_SCALE_RES2                                0x00610b50

#define NV50_PDISPLAY_DAC_MODE_CTRL_P(i)                (0x00610b58 + (i) * 0x8)
#define NV50_PDISPLAY_DAC_MODE_CTRL_C(i)                (0x00610b5c + (i) * 0x8)
#define NV50_PDISPLAY_SOR_MODE_CTRL_P(i)                (0x00610b70 + (i) * 0x8)
#define NV50_PDISPLAY_SOR_MODE_CTRL_C(i)                (0x00610b74 + (i) * 0x8)
#define NV50_PDISPLAY_EXT_MODE_CTRL_P(i)                (0x00610b80 + (i) * 0x8)
#define NV50_PDISPLAY_EXT_MODE_CTRL_C(i)                (0x00610b84 + (i) * 0x8)
#define NV50_PDISPLAY_DAC_MODE_CTRL2_P(i)               (0x00610bdc + (i) * 0x8)
#define NV50_PDISPLAY_DAC_MODE_CTRL2_C(i)               (0x00610be0 + (i) * 0x8)
#define NV90_PDISPLAY_SOR_MODE_CTRL_P(i)                (0x00610794 + (i) * 0x8)
#define NV90_PDISPLAY_SOR_MODE_CTRL_C(i)                (0x00610798 + (i) * 0x8)

#define NV50_PDISPLAY_CRTC_CLK                                       0x00614000
#define NV50_PDISPLAY_CRTC_CLK_CTRL1(i)                 ((i) * 0x800 + 0x614100)
#define NV50_PDISPLAY_CRTC_CLK_CTRL1_CONNECTED                       0x00000600
#define NV50_PDISPLAY_CRTC_CLK_VPLL_A(i)                ((i) * 0x800 + 0x614104)
#define NV50_PDISPLAY_CRTC_CLK_VPLL_B(i)                ((i) * 0x800 + 0x614108)
#define NV50_PDISPLAY_CRTC_CLK_CTRL2(i)                 ((i) * 0x800 + 0x614200)

#define NV50_PDISPLAY_DAC_CLK                                        0x00614000
#define NV50_PDISPLAY_DAC_CLK_CTRL2(i)                  ((i) * 0x800 + 0x614280)

#define NV50_PDISPLAY_SOR_CLK                                        0x00614000
#define NV50_PDISPLAY_SOR_CLK_CTRL2(i)                  ((i) * 0x800 + 0x614300)

#define NV50_PDISPLAY_VGACRTC(r)                                ((r) + 0x619400)

#define NV50_PDISPLAY_DAC                                            0x0061a000
#define NV50_PDISPLAY_DAC_DPMS_CTRL(i)                (0x0061a004 + (i) * 0x800)
#define NV50_PDISPLAY_DAC_DPMS_CTRL_HSYNC_OFF                        0x00000001
#define NV50_PDISPLAY_DAC_DPMS_CTRL_VSYNC_OFF                        0x00000004
#define NV50_PDISPLAY_DAC_DPMS_CTRL_BLANKED                          0x00000010
#define NV50_PDISPLAY_DAC_DPMS_CTRL_OFF                              0x00000040
#define NV50_PDISPLAY_DAC_DPMS_CTRL_PENDING                          0x80000000
#define NV50_PDISPLAY_DAC_LOAD_CTRL(i)                (0x0061a00c + (i) * 0x800)
#define NV50_PDISPLAY_DAC_LOAD_CTRL_ACTIVE                           0x00100000
#define NV50_PDISPLAY_DAC_LOAD_CTRL_PRESENT                          0x38000000
#define NV50_PDISPLAY_DAC_LOAD_CTRL_DONE                             0x80000000
#define NV50_PDISPLAY_DAC_CLK_CTRL1(i)                (0x0061a010 + (i) * 0x800)
#define NV50_PDISPLAY_DAC_CLK_CTRL1_CONNECTED                        0x00000600

#define NV50_PDISPLAY_SOR                                            0x0061c000
#define NV50_PDISPLAY_SOR_DPMS_CTRL(i)                (0x0061c004 + (i) * 0x800)
#define NV50_PDISPLAY_SOR_DPMS_CTRL_PENDING                          0x80000000
#define NV50_PDISPLAY_SOR_DPMS_CTRL_ON                               0x00000001
#define NV50_PDISPLAY_SOR_CLK_CTRL1(i)                (0x0061c008 + (i) * 0x800)
#define NV50_PDISPLAY_SOR_CLK_CTRL1_CONNECTED                        0x00000600
#define NV50_PDISPLAY_SOR_DPMS_STATE(i)               (0x0061c030 + (i) * 0x800)
#define NV50_PDISPLAY_SOR_DPMS_STATE_ACTIVE                          0x00030000
#define NV50_PDISPLAY_SOR_DPMS_STATE_BLANKED                         0x00080000
#define NV50_PDISPLAY_SOR_DPMS_STATE_WAIT                            0x10000000
#define NV50_PDISP_SOR_PWM_DIV(i)                     (0x0061c080 + (i) * 0x800)
#define NV50_PDISP_SOR_PWM_CTL(i)                     (0x0061c084 + (i) * 0x800)
#define NV50_PDISP_SOR_PWM_CTL_NEW                                   0x80000000
#define NVA3_PDISP_SOR_PWM_CTL_UNK                                   0x40000000
#define NV50_PDISP_SOR_PWM_CTL_VAL                                   0x000007ff
#define NVA3_PDISP_SOR_PWM_CTL_VAL                                   0x00ffffff
#define NV50_SOR_DP_CTRL(i, l)           (0x0061c10c + (i) * 0x800 + (l) * 0x80)
#define NV50_SOR_DP_CTRL_ENABLED                                     0x00000001
#define NV50_SOR_DP_CTRL_ENHANCED_FRAME_ENABLED                      0x00004000
#define NV50_SOR_DP_CTRL_LANE_MASK                                   0x001f0000
#define NV50_SOR_DP_CTRL_LANE_0_ENABLED                              0x00010000
#define NV50_SOR_DP_CTRL_LANE_1_ENABLED                              0x00020000
#define NV50_SOR_DP_CTRL_LANE_2_ENABLED                              0x00040000
#define NV50_SOR_DP_CTRL_LANE_3_ENABLED                              0x00080000
#define NV50_SOR_DP_CTRL_TRAINING_PATTERN                            0x0f000000
#define NV50_SOR_DP_CTRL_TRAINING_PATTERN_DISABLED                   0x00000000
#define NV50_SOR_DP_CTRL_TRAINING_PATTERN_1                          0x01000000
#define NV50_SOR_DP_CTRL_TRAINING_PATTERN_2                          0x02000000
#define NV50_SOR_DP_UNK118(i, l)         (0x0061c118 + (i) * 0x800 + (l) * 0x80)
#define NV50_SOR_DP_UNK120(i, l)         (0x0061c120 + (i) * 0x800 + (l) * 0x80)
#define NV50_SOR_DP_SCFG(i, l)           (0x0061c128 + (i) * 0x800 + (l) * 0x80)
#define NV50_SOR_DP_UNK130(i, l)         (0x0061c130 + (i) * 0x800 + (l) * 0x80)

#define NV50_PDISPLAY_USER(i)                        ((i) * 0x1000 + 0x00640000)
#define NV50_PDISPLAY_USER_PUT(i)                    ((i) * 0x1000 + 0x00640000)
#define NV50_PDISPLAY_USER_GET(i)                    ((i) * 0x1000 + 0x00640004)

#define NV50_PDISPLAY_CURSOR_USER                                    0x00647000
#define NV50_PDISPLAY_CURSOR_USER_POS_CTRL(i)        ((i) * 0x1000 + 0x00647080)
#define NV50_PDISPLAY_CURSOR_USER_POS(i)             ((i) * 0x1000 + 0x00647084)

#define NV_04      0x04
#define NV_10      0x10
#define NV_20      0x20
#define NV_30      0x30
#define NV_40      0x40
#define NV_50      0x50
#define NV_C0      0xc0
#define NV_D0      0xd0
#define NV_E0      0xe0

#define NV_PMC_OFFSET               0x00000000
#define NV_PMC_SIZE                 0x00001000

#define NV_PBUS_OFFSET              0x00001000
#define NV_PBUS_SIZE                0x00001000

#define NV_PFIFO_OFFSET             0x00002000
#define NV_PFIFO_SIZE               0x00002000

#define NV_HDIAG_OFFSET             0x00005000
#define NV_HDIAG_SIZE               0x00001000

#define NV_PRAM_OFFSET              0x00006000
#define NV_PRAM_SIZE                0x00001000

#define NV_PVIDEO_OFFSET            0x00008000
#define NV_PVIDEO_SIZE              0x00001000

#define NV_PTIMER_OFFSET            0x00009000
#define NV_PTIMER_SIZE              0x00001000

#define NV_PPM_OFFSET               0x0000A000
#define NV_PPM_SIZE                 0x00001000

#define NV_PTV_OFFSET               0x0000D000
#define NV_PTV_SIZE                 0x00001000

#define NV_PRMVGA_OFFSET            0x000A0000
#define NV_PRMVGA_SIZE              0x00020000

#define NV_PRMVIO0_OFFSET           0x000C0000
#define NV_PRMVIO_SIZE              0x00002000
#define NV_PRMVIO1_OFFSET           0x000C2000

#define NV_PFB_OFFSET               0x00100000
#define NV_PFB_SIZE                 0x00001000

#define NV_PEXTDEV_OFFSET           0x00101000
#define NV_PEXTDEV_SIZE             0x00001000

#define NV_PME_OFFSET               0x00200000
#define NV_PME_SIZE                 0x00001000

#define NV_PROM_OFFSET              0x00300000
#define NV_PROM_SIZE                0x00010000

#define NV_PGRAPH_OFFSET            0x00400000
#define NV_PGRAPH_SIZE              0x00010000

#define NV_PCRTC0_OFFSET            0x00600000
#define NV_PCRTC0_SIZE              0x00002000 /* empirical */

#define NV_PRMCIO0_OFFSET           0x00601000
#define NV_PRMCIO_SIZE              0x00002000
#define NV_PRMCIO1_OFFSET           0x00603000

#define NV50_DISPLAY_OFFSET           0x00610000
#define NV50_DISPLAY_SIZE             0x0000FFFF

#define NV_PRAMDAC0_OFFSET          0x00680000
#define NV_PRAMDAC0_SIZE            0x00002000

#define NV_PRMDIO0_OFFSET           0x00681000
#define NV_PRMDIO_SIZE              0x00002000
#define NV_PRMDIO1_OFFSET           0x00683000

#define NV_PRAMIN_OFFSET            0x00700000
#define NV_PRAMIN_SIZE              0x00100000

#define NV_FIFO_OFFSET              0x00800000
#define NV_FIFO_SIZE                0x00800000

#define NV_PMC_BOOT_0			0x00000000
#define NV_PMC_ENABLE			0x00000200

#define NV_VIO_VSE2			0x000003c3
#define NV_VIO_SRX			0x000003c4

#define NV_CIO_CRX__COLOR		0x000003d4
#define NV_CIO_CR__COLOR		0x000003d5

#define NV_PBUS_DEBUG_1			0x00001084
#define NV_PBUS_DEBUG_4			0x00001098
#define NV_PBUS_DEBUG_DUALHEAD_CTL	0x000010f0
#define NV_PBUS_POWERCTRL_1		0x00001584
#define NV_PBUS_POWERCTRL_2		0x00001588
#define NV_PBUS_POWERCTRL_4		0x00001590
#define NV_PBUS_PCI_NV_19		0x0000184C
#define NV_PBUS_PCI_NV_20		0x00001850
#	define NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED	(0 << 0)
#	define NV_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED	(1 << 0)

#define NV_PFIFO_RAMHT			0x00002210

#define NV_PTV_TV_INDEX			0x0000d220
#define NV_PTV_TV_DATA			0x0000d224
#define NV_PTV_HFILTER			0x0000d310
#define NV_PTV_HFILTER2			0x0000d390
#define NV_PTV_VFILTER			0x0000d510

#define NV_PRMVIO_MISC__WRITE		0x000c03c2
#define NV_PRMVIO_SRX			0x000c03c4
#define NV_PRMVIO_SR			0x000c03c5
#	define NV_VIO_SR_RESET_INDEX		0x00
#	define NV_VIO_SR_CLOCK_INDEX		0x01
#	define NV_VIO_SR_PLANE_MASK_INDEX	0x02
#	define NV_VIO_SR_CHAR_MAP_INDEX		0x03
#	define NV_VIO_SR_MEM_MODE_INDEX		0x04
#define NV_PRMVIO_MISC__READ		0x000c03cc
#define NV_PRMVIO_GRX			0x000c03ce
#define NV_PRMVIO_GX			0x000c03cf
#	define NV_VIO_GX_SR_INDEX		0x00
#	define NV_VIO_GX_SREN_INDEX		0x01
#	define NV_VIO_GX_CCOMP_INDEX		0x02
#	define NV_VIO_GX_ROP_INDEX		0x03
#	define NV_VIO_GX_READ_MAP_INDEX		0x04
#	define NV_VIO_GX_MODE_INDEX		0x05
#	define NV_VIO_GX_MISC_INDEX		0x06
#	define NV_VIO_GX_DONT_CARE_INDEX	0x07
#	define NV_VIO_GX_BIT_MASK_INDEX		0x08

#define NV_PCRTC_INTR_0					0x00600100
#	define NV_PCRTC_INTR_0_VBLANK				(1 << 0)
#define NV_PCRTC_INTR_EN_0				0x00600140
#define NV_PCRTC_START					0x00600800
#define NV_PCRTC_CONFIG					0x00600804
#	define NV_PCRTC_CONFIG_START_ADDRESS_NON_VGA		(1 << 0)
#	define NV04_PCRTC_CONFIG_START_ADDRESS_HSYNC		(4 << 0)
#	define NV10_PCRTC_CONFIG_START_ADDRESS_HSYNC		(2 << 0)
#define NV_PCRTC_CURSOR_CONFIG				0x00600810
#	define NV_PCRTC_CURSOR_CONFIG_ENABLE_ENABLE		(1 << 0)
#	define NV_PCRTC_CURSOR_CONFIG_DOUBLE_SCAN_ENABLE	(1 << 4)
#	define NV_PCRTC_CURSOR_CONFIG_ADDRESS_SPACE_PNVM	(1 << 8)
#	define NV_PCRTC_CURSOR_CONFIG_CUR_BPP_32		(1 << 12)
#	define NV_PCRTC_CURSOR_CONFIG_CUR_PIXELS_64		(1 << 16)
#	define NV_PCRTC_CURSOR_CONFIG_CUR_LINES_32		(2 << 24)
#	define NV_PCRTC_CURSOR_CONFIG_CUR_LINES_64		(4 << 24)
#	define NV_PCRTC_CURSOR_CONFIG_CUR_BLEND_ALPHA		(1 << 28)

/* note: PCRTC_GPIO is not available on nv10, and in fact aliases 0x600810 */
#define NV_PCRTC_GPIO					0x00600818
#define NV_PCRTC_GPIO_EXT				0x0060081c
#define NV_PCRTC_830					0x00600830
#define NV_PCRTC_834					0x00600834
#define NV_PCRTC_850					0x00600850
#define NV_PCRTC_ENGINE_CTRL				0x00600860
#	define NV_CRTC_FSEL_I2C					(1 << 4)
#	define NV_CRTC_FSEL_OVERLAY				(1 << 12)

#define NV_PRMCIO_ARX			0x006013c0
#define NV_PRMCIO_AR__WRITE		0x006013c0
#define NV_PRMCIO_AR__READ		0x006013c1
#	define NV_CIO_AR_MODE_INDEX		0x10
#	define NV_CIO_AR_OSCAN_INDEX		0x11
#	define NV_CIO_AR_PLANE_INDEX		0x12
#	define NV_CIO_AR_HPP_INDEX		0x13
#	define NV_CIO_AR_CSEL_INDEX		0x14
#define NV_PRMCIO_INP0			0x006013c2
#define NV_PRMCIO_CRX__COLOR		0x006013d4
#define NV_PRMCIO_CR__COLOR		0x006013d5
/* Standard VGA CRTC registers */
#	define NV_CIO_CR_HDT_INDEX		0x00	/* horizontal display total */
#	define NV_CIO_CR_HDE_INDEX		0x01	/* horizontal display end */
#	define NV_CIO_CR_HBS_INDEX		0x02	/* horizontal blanking start */
#	define NV_CIO_CR_HBE_INDEX		0x03	/* horizontal blanking end */
#		define NV_CIO_CR_HBE_4_0		4:0
#	define NV_CIO_CR_HRS_INDEX		0x04	/* horizontal retrace start */
#	define NV_CIO_CR_HRE_INDEX		0x05	/* horizontal retrace end */
#		define NV_CIO_CR_HRE_4_0		4:0
#		define NV_CIO_CR_HRE_HBE_5		7:7
#	define NV_CIO_CR_VDT_INDEX		0x06	/* vertical display total */
#	define NV_CIO_CR_OVL_INDEX		0x07	/* overflow bits */
#		define NV_CIO_CR_OVL_VDT_8		0:0
#		define NV_CIO_CR_OVL_VDE_8		1:1
#		define NV_CIO_CR_OVL_VRS_8		2:2
#		define NV_CIO_CR_OVL_VBS_8		3:3
#		define NV_CIO_CR_OVL_VDT_9		5:5
#		define NV_CIO_CR_OVL_VDE_9		6:6
#		define NV_CIO_CR_OVL_VRS_9		7:7
#	define NV_CIO_CR_RSAL_INDEX		0x08	/* normally "preset row scan" */
#	define NV_CIO_CR_CELL_HT_INDEX		0x09	/* cell height?! normally "max scan line" */
#		define NV_CIO_CR_CELL_HT_VBS_9		5:5
#		define NV_CIO_CR_CELL_HT_SCANDBL	7:7
#	define NV_CIO_CR_CURS_ST_INDEX		0x0a	/* cursor start */
#	define NV_CIO_CR_CURS_END_INDEX		0x0b	/* cursor end */
#	define NV_CIO_CR_SA_HI_INDEX		0x0c	/* screen start address high */
#	define NV_CIO_CR_SA_LO_INDEX		0x0d	/* screen start address low */
#	define NV_CIO_CR_TCOFF_HI_INDEX		0x0e	/* cursor offset high */
#	define NV_CIO_CR_TCOFF_LO_INDEX		0x0f	/* cursor offset low */
#	define NV_CIO_CR_VRS_INDEX		0x10	/* vertical retrace start */
#	define NV_CIO_CR_VRE_INDEX		0x11	/* vertical retrace end */
#		define NV_CIO_CR_VRE_3_0		3:0
#	define NV_CIO_CR_VDE_INDEX		0x12	/* vertical display end */
#	define NV_CIO_CR_OFFSET_INDEX		0x13	/* sets screen pitch */
#	define NV_CIO_CR_ULINE_INDEX		0x14	/* underline location */
#	define NV_CIO_CR_VBS_INDEX		0x15	/* vertical blank start */
#	define NV_CIO_CR_VBE_INDEX		0x16	/* vertical blank end */
#	define NV_CIO_CR_MODE_INDEX		0x17	/* crtc mode control */
#	define NV_CIO_CR_LCOMP_INDEX		0x18	/* line compare */
/* Extended VGA CRTC registers */
#	define NV_CIO_CRE_RPC0_INDEX		0x19	/* repaint control 0 */
#		define NV_CIO_CRE_RPC0_OFFSET_10_8	7:5
#	define NV_CIO_CRE_RPC1_INDEX		0x1a	/* repaint control 1 */
#		define NV_CIO_CRE_RPC1_LARGE		2:2
#	define NV_CIO_CRE_FF_INDEX		0x1b	/* fifo control */
#	define NV_CIO_CRE_ENH_INDEX		0x1c	/* enhanced? */
#	define NV_CIO_SR_LOCK_INDEX		0x1f	/* crtc lock */
#		define NV_CIO_SR_UNLOCK_RW_VALUE	0x57
#		define NV_CIO_SR_LOCK_VALUE		0x99
#	define NV_CIO_CRE_FFLWM__INDEX		0x20	/* fifo low water mark */
#	define NV_CIO_CRE_21			0x21	/* vga shadow crtc lock */
#	define NV_CIO_CRE_LSR_INDEX		0x25	/* ? */
#		define NV_CIO_CRE_LSR_VDT_10		0:0
#		define NV_CIO_CRE_LSR_VDE_10		1:1
#		define NV_CIO_CRE_LSR_VRS_10		2:2
#		define NV_CIO_CRE_LSR_VBS_10		3:3
#		define NV_CIO_CRE_LSR_HBE_6		4:4
#	define NV_CIO_CR_ARX_INDEX		0x26	/* attribute index -- ro copy of 0x60.3c0 */
#	define NV_CIO_CRE_CHIP_ID_INDEX		0x27	/* chip revision */
#	define NV_CIO_CRE_PIXEL_INDEX		0x28
#		define NV_CIO_CRE_PIXEL_FORMAT		1:0
#	define NV_CIO_CRE_HEB__INDEX		0x2d	/* horizontal extra bits? */
#		define NV_CIO_CRE_HEB_HDT_8		0:0
#		define NV_CIO_CRE_HEB_HDE_8		1:1
#		define NV_CIO_CRE_HEB_HBS_8		2:2
#		define NV_CIO_CRE_HEB_HRS_8		3:3
#		define NV_CIO_CRE_HEB_ILC_8		4:4
#	define NV_CIO_CRE_2E			0x2e	/* some scratch or dummy reg to force writes to sink in */
#	define NV_CIO_CRE_HCUR_ADDR2_INDEX	0x2f	/* cursor */
#	define NV_CIO_CRE_HCUR_ADDR0_INDEX	0x30		/* pixmap */
#		define NV_CIO_CRE_HCUR_ADDR0_ADR	6:0
#		define NV_CIO_CRE_HCUR_ASI		7:7
#	define NV_CIO_CRE_HCUR_ADDR1_INDEX	0x31			/* address */
#		define NV_CIO_CRE_HCUR_ADDR1_ENABLE	0:0
#		define NV_CIO_CRE_HCUR_ADDR1_CUR_DBL	1:1
#		define NV_CIO_CRE_HCUR_ADDR1_ADR	7:2
#	define NV_CIO_CRE_LCD__INDEX		0x33
#		define NV_CIO_CRE_LCD_LCD_SELECT	0:0
#		define NV_CIO_CRE_LCD_ROUTE_MASK	0x3b
#	define NV_CIO_CRE_DDC0_STATUS__INDEX	0x36
#	define NV_CIO_CRE_DDC0_WR__INDEX	0x37
#	define NV_CIO_CRE_ILACE__INDEX		0x39	/* interlace */
#	define NV_CIO_CRE_SCRATCH3__INDEX	0x3b
#	define NV_CIO_CRE_SCRATCH4__INDEX	0x3c
#	define NV_CIO_CRE_DDC_STATUS__INDEX	0x3e
#	define NV_CIO_CRE_DDC_WR__INDEX		0x3f
#	define NV_CIO_CRE_EBR_INDEX		0x41	/* extra bits ? (vertical) */
#		define NV_CIO_CRE_EBR_VDT_11		0:0
#		define NV_CIO_CRE_EBR_VDE_11		2:2
#		define NV_CIO_CRE_EBR_VRS_11		4:4
#		define NV_CIO_CRE_EBR_VBS_11		6:6
#	define NV_CIO_CRE_42			0x42
#		define NV_CIO_CRE_42_OFFSET_11		6:6
#	define NV_CIO_CRE_43			0x43
#	define NV_CIO_CRE_44			0x44	/* head control */
#	define NV_CIO_CRE_CSB			0x45	/* colour saturation boost */
#	define NV_CIO_CRE_RCR			0x46
#		define NV_CIO_CRE_RCR_ENDIAN_BIG	7:7
#	define NV_CIO_CRE_47			0x47	/* extended fifo lwm, used on nv30+ */
#	define NV_CIO_CRE_49			0x49
#	define NV_CIO_CRE_4B			0x4b	/* given patterns in 0x[2-3][a-c] regs, probably scratch 6 */
#	define NV_CIO_CRE_TVOUT_LATENCY		0x52
#	define NV_CIO_CRE_53			0x53	/* `fp_htiming' according to Haiku */
#	define NV_CIO_CRE_54			0x54	/* `fp_vtiming' according to Haiku */
#	define NV_CIO_CRE_57			0x57	/* index reg for cr58 */
#	define NV_CIO_CRE_58			0x58	/* data reg for cr57 */
#	define NV_CIO_CRE_59			0x59	/* related to on/off-chip-ness of digital outputs */
#	define NV_CIO_CRE_5B			0x5B	/* newer colour saturation reg */
#	define NV_CIO_CRE_85			0x85
#	define NV_CIO_CRE_86			0x86
#define NV_PRMCIO_INP0__COLOR		0x006013da

#define NV_PRAMDAC_CU_START_POS				0x00680300
#	define NV_PRAMDAC_CU_START_POS_X			15:0
#	define NV_PRAMDAC_CU_START_POS_Y			31:16
#define NV_RAMDAC_NV10_CURSYNC				0x00680404

#define NV_PRAMDAC_NVPLL_COEFF				0x00680500
#define NV_PRAMDAC_MPLL_COEFF				0x00680504
#define NV_PRAMDAC_VPLL_COEFF				0x00680508
#	define NV30_RAMDAC_ENABLE_VCO2				(8 << 4)

#define NV_PRAMDAC_PLL_COEFF_SELECT			0x0068050c
#	define NV_PRAMDAC_PLL_COEFF_SELECT_USE_VPLL2_TRUE	(4 << 0)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_SOURCE_PROG_MPLL	(1 << 8)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_SOURCE_PROG_VPLL	(2 << 8)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_SOURCE_PROG_NVPLL	(4 << 8)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_PLL_SOURCE_VPLL2	(8 << 8)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_TV_VSCLK1		(1 << 16)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_TV_PCLK1		(2 << 16)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_TV_VSCLK2		(4 << 16)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_TV_PCLK2		(8 << 16)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_TV_CLK_SOURCE_VIP	(1 << 20)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_VCLK_RATIO_DB2	(1 << 28)
#	define NV_PRAMDAC_PLL_COEFF_SELECT_VCLK2_RATIO_DB2	(2 << 28)

#define NV_PRAMDAC_PLL_SETUP_CONTROL			0x00680510
#define NV_RAMDAC_VPLL2					0x00680520
#define NV_PRAMDAC_SEL_CLK				0x00680524
#define NV_RAMDAC_DITHER_NV11				0x00680528
#define NV_PRAMDAC_DACCLK				0x0068052c
#	define NV_PRAMDAC_DACCLK_SEL_DACCLK			(1 << 0)

#define NV_RAMDAC_NVPLL_B				0x00680570
#define NV_RAMDAC_MPLL_B				0x00680574
#define NV_RAMDAC_VPLL_B				0x00680578
#define NV_RAMDAC_VPLL2_B				0x0068057c
#	define NV31_RAMDAC_ENABLE_VCO2				(8 << 28)
#define NV_PRAMDAC_580					0x00680580
#	define NV_RAMDAC_580_VPLL1_ACTIVE			(1 << 8)
#	define NV_RAMDAC_580_VPLL2_ACTIVE			(1 << 28)

#define NV_PRAMDAC_GENERAL_CONTROL			0x00680600
#	define NV_PRAMDAC_GENERAL_CONTROL_PIXMIX_ON		(3 << 4)
#	define NV_PRAMDAC_GENERAL_CONTROL_VGA_STATE_SEL		(1 << 8)
#	define NV_PRAMDAC_GENERAL_CONTROL_ALT_MODE_SEL		(1 << 12)
#	define NV_PRAMDAC_GENERAL_CONTROL_TERMINATION_75OHM	(2 << 16)
#	define NV_PRAMDAC_GENERAL_CONTROL_BPC_8BITS		(1 << 20)
#	define NV_PRAMDAC_GENERAL_CONTROL_PIPE_LONG		(2 << 28)
#define NV_PRAMDAC_TEST_CONTROL				0x00680608
#	define NV_PRAMDAC_TEST_CONTROL_TP_INS_EN_ASSERTED	(1 << 12)
#	define NV_PRAMDAC_TEST_CONTROL_PWRDWN_DAC_OFF		(1 << 16)
#	define NV_PRAMDAC_TEST_CONTROL_SENSEB_ALLHI		(1 << 28)
#define NV_PRAMDAC_TESTPOINT_DATA			0x00680610
#	define NV_PRAMDAC_TESTPOINT_DATA_NOTBLANK		(8 << 28)
#define NV_PRAMDAC_630					0x00680630
#define NV_PRAMDAC_634					0x00680634

#define NV_PRAMDAC_TV_SETUP				0x00680700
#define NV_PRAMDAC_TV_VTOTAL				0x00680720
#define NV_PRAMDAC_TV_VSKEW				0x00680724
#define NV_PRAMDAC_TV_VSYNC_DELAY			0x00680728
#define NV_PRAMDAC_TV_HTOTAL				0x0068072c
#define NV_PRAMDAC_TV_HSKEW				0x00680730
#define NV_PRAMDAC_TV_HSYNC_DELAY			0x00680734
#define NV_PRAMDAC_TV_HSYNC_DELAY2			0x00680738

#define NV_PRAMDAC_TV_SETUP                             0x00680700

#define NV_PRAMDAC_FP_VDISPLAY_END			0x00680800
#define NV_PRAMDAC_FP_VTOTAL				0x00680804
#define NV_PRAMDAC_FP_VCRTC				0x00680808
#define NV_PRAMDAC_FP_VSYNC_START			0x0068080c
#define NV_PRAMDAC_FP_VSYNC_END				0x00680810
#define NV_PRAMDAC_FP_VVALID_START			0x00680814
#define NV_PRAMDAC_FP_VVALID_END			0x00680818
#define NV_PRAMDAC_FP_HDISPLAY_END			0x00680820
#define NV_PRAMDAC_FP_HTOTAL				0x00680824
#define NV_PRAMDAC_FP_HCRTC				0x00680828
#define NV_PRAMDAC_FP_HSYNC_START			0x0068082c
#define NV_PRAMDAC_FP_HSYNC_END				0x00680830
#define NV_PRAMDAC_FP_HVALID_START			0x00680834
#define NV_PRAMDAC_FP_HVALID_END			0x00680838

#define NV_RAMDAC_FP_DITHER				0x0068083c
#define NV_PRAMDAC_FP_TG_CONTROL			0x00680848
#	define NV_PRAMDAC_FP_TG_CONTROL_VSYNC_POS		(1 << 0)
#	define NV_PRAMDAC_FP_TG_CONTROL_VSYNC_DISABLE		(2 << 0)
#	define NV_PRAMDAC_FP_TG_CONTROL_HSYNC_POS		(1 << 4)
#	define NV_PRAMDAC_FP_TG_CONTROL_HSYNC_DISABLE		(2 << 4)
#	define NV_PRAMDAC_FP_TG_CONTROL_MODE_SCALE		(0 << 8)
#	define NV_PRAMDAC_FP_TG_CONTROL_MODE_CENTER		(1 << 8)
#	define NV_PRAMDAC_FP_TG_CONTROL_MODE_NATIVE		(2 << 8)
#	define NV_PRAMDAC_FP_TG_CONTROL_READ_PROG		(1 << 20)
#	define NV_PRAMDAC_FP_TG_CONTROL_WIDTH_12		(1 << 24)
#	define NV_PRAMDAC_FP_TG_CONTROL_DISPEN_POS		(1 << 28)
#	define NV_PRAMDAC_FP_TG_CONTROL_DISPEN_DISABLE		(2 << 28)
#define NV_PRAMDAC_FP_MARGIN_COLOR			0x0068084c
#define NV_PRAMDAC_850					0x00680850
#define NV_PRAMDAC_85C					0x0068085c
#define NV_PRAMDAC_FP_DEBUG_0				0x00680880
#	define NV_PRAMDAC_FP_DEBUG_0_XSCALE_ENABLE		(1 << 0)
#	define NV_PRAMDAC_FP_DEBUG_0_YSCALE_ENABLE		(1 << 4)
/* This doesn't seem to be essential for tmds, but still often set */
#	define NV_RAMDAC_FP_DEBUG_0_TMDS_ENABLED		(8 << 4)
#	define NV_PRAMDAC_FP_DEBUG_0_XINTERP_BILINEAR		(1 << 8)
#	define NV_PRAMDAC_FP_DEBUG_0_YINTERP_BILINEAR		(1 << 12)
#	define NV_PRAMDAC_FP_DEBUG_0_XWEIGHT_ROUND		(1 << 20)
#	define NV_PRAMDAC_FP_DEBUG_0_YWEIGHT_ROUND		(1 << 24)
#       define NV_PRAMDAC_FP_DEBUG_0_PWRDOWN_FPCLK              (1 << 28)
#define NV_PRAMDAC_FP_DEBUG_1				0x00680884
#	define NV_PRAMDAC_FP_DEBUG_1_XSCALE_VALUE		11:0
#	define NV_PRAMDAC_FP_DEBUG_1_XSCALE_TESTMODE_ENABLE	(1 << 12)
#	define NV_PRAMDAC_FP_DEBUG_1_YSCALE_VALUE		27:16
#	define NV_PRAMDAC_FP_DEBUG_1_YSCALE_TESTMODE_ENABLE	(1 << 28)
#define NV_PRAMDAC_FP_DEBUG_2				0x00680888
#define NV_PRAMDAC_FP_DEBUG_3				0x0068088C

/* see NV_PRAMDAC_INDIR_TMDS in rules.xml */
#define NV_PRAMDAC_FP_TMDS_CONTROL			0x006808b0
#	define NV_PRAMDAC_FP_TMDS_CONTROL_WRITE_DISABLE		(1 << 16)
#define NV_PRAMDAC_FP_TMDS_DATA				0x006808b4

#define NV_PRAMDAC_8C0                                  0x006808c0

/* Some kind of switch */
#define NV_PRAMDAC_900					0x00680900
#define NV_PRAMDAC_A20					0x00680A20
#define NV_PRAMDAC_A24					0x00680A24
#define NV_PRAMDAC_A34					0x00680A34

#define NV_PRAMDAC_CTV					0x00680c00

/* names fabricated from NV_USER_DAC info */
#define NV_PRMDIO_PIXEL_MASK		0x006813c6
#	define NV_PRMDIO_PIXEL_MASK_MASK	0xff
#define NV_PRMDIO_READ_MODE_ADDRESS	0x006813c7
#define NV_PRMDIO_WRITE_MODE_ADDRESS	0x006813c8
#define NV_PRMDIO_PALETTE_DATA		0x006813c9

#define NV_PGRAPH_DEBUG_0		0x00400080
#define NV_PGRAPH_DEBUG_1		0x00400084
#define NV_PGRAPH_DEBUG_2_NV04		0x00400088
#define NV_PGRAPH_DEBUG_2		0x00400620
#define NV_PGRAPH_DEBUG_3		0x0040008c
#define NV_PGRAPH_DEBUG_4		0x00400090
#define NV_PGRAPH_INTR			0x00400100
#define NV_PGRAPH_INTR_EN		0x00400140
#define NV_PGRAPH_CTX_CONTROL		0x00400144
#define NV_PGRAPH_CTX_CONTROL_NV04	0x00400170
#define NV_PGRAPH_ABS_UCLIP_XMIN	0x0040053C
#define NV_PGRAPH_ABS_UCLIP_YMIN	0x00400540
#define NV_PGRAPH_ABS_UCLIP_XMAX	0x00400544
#define NV_PGRAPH_ABS_UCLIP_YMAX	0x00400548
#define NV_PGRAPH_BETA_AND		0x00400608
#define NV_PGRAPH_LIMIT_VIOL_PIX	0x00400610
#define NV_PGRAPH_BOFFSET0		0x00400640
#define NV_PGRAPH_BOFFSET1		0x00400644
#define NV_PGRAPH_BOFFSET2		0x00400648
#define NV_PGRAPH_BLIMIT0		0x00400684
#define NV_PGRAPH_BLIMIT1		0x00400688
#define NV_PGRAPH_BLIMIT2		0x0040068c
#define NV_PGRAPH_STATUS		0x00400700
#define NV_PGRAPH_SURFACE		0x00400710
#define NV_PGRAPH_STATE			0x00400714
#define NV_PGRAPH_FIFO			0x00400720
#define NV_PGRAPH_PATTERN_SHAPE		0x00400810
#define NV_PGRAPH_TILE			0x00400b00

#define NV_PVIDEO_INTR_EN		0x00008140
#define NV_PVIDEO_BUFFER		0x00008700
#define NV_PVIDEO_STOP			0x00008704
#define NV_PVIDEO_UVPLANE_BASE(buff)	(0x00008800+(buff)*4)
#define NV_PVIDEO_UVPLANE_LIMIT(buff)	(0x00008808+(buff)*4)
#define NV_PVIDEO_UVPLANE_OFFSET_BUFF(buff)	(0x00008820+(buff)*4)
#define NV_PVIDEO_BASE(buff)		(0x00008900+(buff)*4)
#define NV_PVIDEO_LIMIT(buff)		(0x00008908+(buff)*4)
#define NV_PVIDEO_LUMINANCE(buff)	(0x00008910+(buff)*4)
#define NV_PVIDEO_CHROMINANCE(buff)	(0x00008918+(buff)*4)
#define NV_PVIDEO_OFFSET_BUFF(buff)	(0x00008920+(buff)*4)
#define NV_PVIDEO_SIZE_IN(buff)		(0x00008928+(buff)*4)
#define NV_PVIDEO_POINT_IN(buff)	(0x00008930+(buff)*4)
#define NV_PVIDEO_DS_DX(buff)		(0x00008938+(buff)*4)
#define NV_PVIDEO_DT_DY(buff)		(0x00008940+(buff)*4)
#define NV_PVIDEO_POINT_OUT(buff)	(0x00008948+(buff)*4)
#define NV_PVIDEO_SIZE_OUT(buff)	(0x00008950+(buff)*4)
#define NV_PVIDEO_FORMAT(buff)		(0x00008958+(buff)*4)
#	define NV_PVIDEO_FORMAT_PLANAR			(1 << 0)
#	define NV_PVIDEO_FORMAT_COLOR_LE_CR8YB8CB8YA8	(1 << 16)
#	define NV_PVIDEO_FORMAT_DISPLAY_COLOR_KEY	(1 << 20)
#	define NV_PVIDEO_FORMAT_MATRIX_ITURBT709	(1 << 24)
#define NV_PVIDEO_COLOR_KEY		0x00008B00

/* NV04 overlay defines from VIDIX & Haiku */
#define NV_PVIDEO_INTR_EN_0		0x00680140
#define NV_PVIDEO_STEP_SIZE		0x00680200
#define NV_PVIDEO_CONTROL_Y		0x00680204
#define NV_PVIDEO_CONTROL_X		0x00680208
#define NV_PVIDEO_BUFF0_START_ADDRESS	0x0068020c
#define NV_PVIDEO_BUFF0_PITCH_LENGTH	0x00680214
#define NV_PVIDEO_BUFF0_OFFSET		0x0068021c
#define NV_PVIDEO_BUFF1_START_ADDRESS	0x00680210
#define NV_PVIDEO_BUFF1_PITCH_LENGTH	0x00680218
#define NV_PVIDEO_BUFF1_OFFSET		0x00680220
#define NV_PVIDEO_OE_STATE		0x00680224
#define NV_PVIDEO_SU_STATE		0x00680228
#define NV_PVIDEO_RM_STATE		0x0068022c
#define NV_PVIDEO_WINDOW_START		0x00680230
#define NV_PVIDEO_WINDOW_SIZE		0x00680234
#define NV_PVIDEO_FIFO_THRES_SIZE	0x00680238
#define NV_PVIDEO_FIFO_BURST_LENGTH	0x0068023c
#define NV_PVIDEO_KEY			0x00680240
#define NV_PVIDEO_OVERLAY		0x00680244
#define NV_PVIDEO_RED_CSC_OFFSET	0x00680280
#define NV_PVIDEO_GREEN_CSC_OFFSET	0x00680284
#define NV_PVIDEO_BLUE_CSC_OFFSET	0x00680288
#define NV_PVIDEO_CSC_ADJUST		0x0068028c

#define ROM16(x) OSSwapLittleToHostInt16(*(UInt16 *)&(x))
#define ROM32(x) OSSwapLittleToHostInt32(*(UInt32 *)&(x))
#define ROMPTR(d,x) ({            \
ROM16(x) ? &d[ROM16(x)] : NULL; \
})

#define NouveauDebugLog(string, args...)	do { if (0) { IOLog ("%s: [Debug] " string "\n", "NouveauSensors:" , ## args); } } while(0)
#define NouveauWarningLog(string, args...) do { IOLog ("%s: [Warning] " string "\n", "NouveauSensors:" , ## args); } while(0)
#define NouveauInfoLog(string, args...)	do { IOLog ("%s: " string "\n", "NouveauSensors:" , ## args); } while(0)

#endif
