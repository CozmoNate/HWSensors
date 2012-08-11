//
//  nouveau_bios.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nouveau_bios_h
#define HWSensors_nouveau_bios_h

#include "nouveau_definitions.h"

#define DCB_MAX_NUM_ENTRIES 16
#define DCB_MAX_NUM_I2C_ENTRIES 16
#define DCB_MAX_NUM_GPIO_ENTRIES 32
#define DCB_MAX_NUM_CONNECTOR_ENTRIES 16

#define DCB_LOC_ON_CHIP 0

#define ROM16(x) OSSwapLittleToHostInt16(*(u16 *)&(x))
#define ROM32(x) OSSwapLittleToHostInt32(*(u32 *)&(x))
#define ROM48(x) ({ u8 *p = &(x); (u64)ROM16(p[4]) << 32 | ROM32(p[0]); })
#define ROM64(x) OSSwapLittleToHostInt64(*(u64 *)&(x))
#define ROMPTR(d,x) ({            \
ROM16(x) ? &d->vbios.data[ROM16(x)] : NULL; \
})

struct bit_entry {
	uint8_t  id;
	uint8_t  version;
	uint16_t length;
	uint16_t offset;
	uint8_t *data;
};

struct nouveau_bios_version {
    u8 major;
    u8 chip;
    u8 minor;
    u8 micro;
};

struct nouveau_bios {
	u32 size;
	u8 *data;
    
	u32 bmp_offset;
	u32 bit_offset;
    
    nouveau_bios_version version;
};

enum dcb_output_type {
	DCB_OUTPUT_ANALOG	= 0x0,
	DCB_OUTPUT_TV		= 0x1,
	DCB_OUTPUT_TMDS		= 0x2,
	DCB_OUTPUT_LVDS		= 0x3,
	DCB_OUTPUT_DP		= 0x6,
	DCB_OUTPUT_EOL		= 0xe,
	DCB_OUTPUT_UNUSED	= 0xf,
	DCB_OUTPUT_ANY = -1,
};

struct sor_conf {
    int link;
};

struct dcb_output {
	int index;	/* may not be raw dcb index if merging has happened */
	enum dcb_output_type type;
	uint8_t i2c_index;
	uint8_t heads;
	uint8_t connector;
	uint8_t bus;
	uint8_t location;
	uint8_t or_or;
	bool duallink_possible;
	union {
		struct sor_conf sorconf;
		struct {
			int maxfreq;
		} crtconf;
		struct {
			struct sor_conf sor;
			bool use_straps_for_mode;
			bool use_acpi_for_edid;
			bool use_power_scripts;
		} lvdsconf;
		struct {
			bool has_component_output;
		} tvconf;
		struct {
			struct sor_conf sor;
			int link_nr;
			int link_bw;
		} dpconf;
		struct {
			struct sor_conf sor;
			int slave_addr;
		} tmdsconf;
	};
	bool i2c_upper_default;
};

struct dcb_table {
	uint8_t version;
	int entries;
	struct dcb_output entry[DCB_MAX_NUM_ENTRIES];
};

enum nvbios_type {
    NVBIOS_BMP,
    NVBIOS_BIT
};

struct nvbios {
	enum nvbios_type type;
	uint16_t offset;
	uint32_t length;
	uint8_t *data;
    
	uint8_t chip_version;
    
	uint32_t dactestval;
	uint32_t tvdactestval;
	uint8_t digital_min_front_porch;
	bool fp_no_ddc;
    
	//spinlock_t lock;
    
	bool execute;
    
	uint8_t major_version;
	uint8_t feature_byte;
	bool is_mobile;
    
	uint32_t fmaxvco, fminvco;
    
	bool old_style_init;
	uint16_t init_script_tbls_ptr;
	uint16_t extra_init_script_tbl_ptr;
	uint16_t macro_index_tbl_ptr;
	uint16_t macro_tbl_ptr;
	uint16_t condition_tbl_ptr;
	uint16_t io_condition_tbl_ptr;
	uint16_t io_flag_condition_tbl_ptr;
	uint16_t init_function_tbl_ptr;
    
	uint16_t pll_limit_tbl_ptr;
	uint16_t ram_restrict_tbl_ptr;
	uint8_t ram_restrict_group_count;
    
	uint16_t some_script_ptr; /* BIT I + 14 */
	uint16_t init96_tbl_ptr; /* BIT I + 16 */
    
	struct dcb_table dcb;
    
	struct {
		int crtchead;
	} state;
    
	struct {
		struct dcb_output *output;
		int crtc;
		uint16_t script_table_ptr;
	} display;
    
	struct {
		uint16_t fptablepointer;	/* also used by tmds */
		uint16_t fpxlatetableptr;
		int xlatwidth;
		uint16_t lvdsmanufacturerpointer;
		uint16_t fpxlatemanufacturertableptr;
		uint16_t mode_ptr;
		uint16_t xlated_entry;
		bool power_off_for_reset;
		bool reset_after_pclk_change;
		bool dual_link;
		bool link_c_increment;
		bool if_is_24bit;
		int duallink_transition_clk;
		uint8_t strapless_is_24bit;
		uint8_t *edid;
        
		/* will need resetting after suspend */
		int last_script_invoc;
		bool lvds_init_run;
	} fp;
    
	struct {
		uint16_t output0_script_ptr;
		uint16_t output1_script_ptr;
	} tmds;
    
	struct {
		uint16_t mem_init_tbl_ptr;
		uint16_t sdr_seq_tbl_ptr;
		uint16_t ddr_seq_tbl_ptr;
        
		struct {
			uint8_t crt, tv, panel;
		} i2c_indices;
        
		uint16_t lvds_single_a_script_ptr;
	} legacy;
};

u8 nv_ro08(struct nouveau_device *device, u32 addr);
u16 nv_ro16(struct nouveau_device *device, u32 addr);
u32 nv_ro32(struct nouveau_device *device, u32 addr);
void nv_wo08(struct nouveau_device *device, u32 addr, u8 data);
void nv_wo16(struct nouveau_device *device, u32 addr, u16 data);
void nv_wo32(struct nouveau_device *device, u32 addr, u32 data);

u16 nouveau_dcb_table(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *cnt, u8 *len);

bool nouveau_bit_table(struct nouveau_device *, u8 id, struct bit_entry *);

bool nouveau_bios_shadow(struct nouveau_device *device);
void nouveau_bios_parse(struct nouveau_device *device);

#endif
