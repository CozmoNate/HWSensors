#ifndef _SUPERIONUB_H
#define _SUPERIONUB_H

#include <IOKit/IOService.h>
#include <architecture/i386/pio.h>

#define kSuperIOHWMAddress  "monitor-address"
#define kSuperIOControlPort "control-port"
#define kSuperIOModelValue  "model"

#define kSuperIOModelName   "model-name"
#define kSuperIOVendorName  "vendor-name"

// Entering ports
const UInt8 kSuperIOPorts[]               = {0x2e, 0x4e};

// Registers
const UInt8 kSuperIOConfigControlRegister = 0x02;
const UInt8 kSuperIOChipIDRegister        = 0x20;
const UInt8 kSuperIOBaseAddressRegister   = 0x60;
const UInt8 kSuperIODeviceSelectRegister  = 0x07;

// Logical device number
const UInt8 kWinbondHardwareMonitorLDN    = 0x0B;
const UInt8 kF71858HardwareMonitorLDN     = 0x02;
const UInt8 kFintekITEHardwareMonitorLDN  = 0x04;

enum SuperIOModel
{
    // ITE
	IT8512F     = 0x8512,
    IT8712F     = 0x8712,
    IT8716F     = 0x8716,
    IT8718F     = 0x8718,
    IT8720F     = 0x8720,
    IT8721F     = 0x8721,
    IT8726F     = 0x8726,
	IT8728F     = 0x8728,
	IT8752F     = 0x8752,
    IT8771E     = 0x8771,
    IT8772E     = 0x8772,
    
    // Winbond
    W83627DHG	= 0xA020,
	W83627UHG	= 0xA230,
    W83627DHGP	= 0xB070,
    W83627EHF	= 0x8800,    
    W83627HF	= 0x5200,
	W83627THF	= 0x8280,
	W83627SF	= 0x5950,
	W83637HF	= 0x7080,
    W83667HG	= 0xA510,
    W83667HGB	= 0xB350,
    W83687THF	= 0x8541,
	W83697HF	= 0x6010,
	W83697SF	= 0x6810,
    
    // Fintek
    F71858		= 0x0507,
    F71862		= 0x0601, 
    F71869		= 0x0814,
    F71869A     = 0x1007,
    F71882		= 0x0541,
    F71889AD    = 0x1005,
    F71889ED	= 0x0909,
    F71889F		= 0x0723,
	F71808		= 0x0901,
    
    
    // Nuvoton
    NCT6771F    = 0xB470,
    NCT6776F    = 0xC330
};

inline UInt8 superio_listen_port_byte(i386_ioport_t port, UInt8 reg)
{
	outb(port, reg);
	return inb(port + 1);
}

inline UInt16 superio_listen_port_word(i386_ioport_t port, UInt8 reg)
{
	return ((superio_listen_port_byte(port, reg) << 8) | superio_listen_port_byte(port, reg + 1));
}

inline void superio_select_logical_device(i386_ioport_t port, UInt8 reg)
{
	outb(port, kSuperIODeviceSelectRegister);
	outb(port + 1, reg);
}

inline void ite_family_enter(i386_ioport_t port)
{
    outb(port, 0x87);
	outb(port, 0x01);
	outb(port, 0x55);
	outb(port, 0x55);
}

inline void ite_family_exit(i386_ioport_t port)
{
    outb(port, kSuperIOConfigControlRegister);
	outb(port + 1, 0x02);
}

inline void winbond_family_enter(i386_ioport_t port)
{
    outb(port, 0x87);
	outb(port, 0x87);
}

inline void winbond_family_exit(i386_ioport_t port)
{
    outb(port, 0xAA);
}

inline const char* superio_get_model_name(UInt16 model)
{
    switch (model) {
        case IT8512F: return "IT8512F";
        case IT8712F: return "IT8712F";
        case IT8716F: return "IT8716F";
        case IT8718F: return "IT8718F";
        case IT8720F: return "IT8720F";
        case IT8721F: return "IT8721F";
        case IT8726F: return "IT8726F";
        case IT8728F: return "IT8728F";
        case IT8752F: return "IT8752F";
        case IT8771E: return "IT8771E";
        case IT8772E: return "IT8772E";
            
        case W83627DHG: return "W83627DHG";
        case W83627UHG: return "W83627UHG";
        case W83627DHGP: return "W83627DHGP";
        case W83627EHF: return "W83627EHF";
        case W83627HF: return "W83627HF";
        case W83627THF: return "W83627THF";
        case W83627SF: return "W83627SF";
        case W83637HF: return "W83637HF";
        case W83667HG: return "W83667HG";
        case W83667HGB: return "W83667HGB";
        case W83687THF: return "W83687THF";
        case W83697HF: return "W83697HF";
        case W83697SF: return "W83697SF";
            
        case F71858: return "F71858";
        case F71862: return "F71862";
        case F71869: return "F71869";
        case F71869A: return "F71869A";
        case F71882: return "F71882";
        case F71889AD: return "F71889AD";
        case F71889ED: return "F71889ED";
        case F71889F: return "f71889F";
        case F71808: return "f71808";
            
        case NCT6771F: return "NCT6771F";
        case NCT6776F: return "NCT6776F";
    }
    
    return "unknown";
}

class SuperIO : public IOService
{
	OSDeclareDefaultStructors(SuperIO)
	
public:
    virtual bool		init(OSDictionary *dictionary = 0);
    virtual IOService	*probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
    virtual void        stop(IOService *provider);
    virtual void		free(void);
};

#endif