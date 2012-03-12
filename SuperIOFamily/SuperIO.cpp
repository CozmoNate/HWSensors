#include "SuperIO.h"

#include <IOKit/IOLib.h>

#define super IOService
OSDefineMetaClassAndStructors(SuperIO, IOService)

bool SuperIO::init(OSDictionary *dictionary)
{	
	if (!super::init(dictionary))
		return false;
    
	return true;
}

IOService *SuperIO::probe(IOService *provider, SInt32 *score)
{
    return super::probe(provider, score);
}

bool SuperIO::start(IOService *provider)
{
	if (!super::start(provider)) return false;
    
    const UInt8 ports[] = {0x2e, 0x4e};
    
    for (int i = 0; i < 2; i++) {
        i386_ioport_t port = ports[i];
        
        winbond_family_enter(port);
        
        UInt16 id = superio_listen_port_word(port, kSuperIOChipIDRegister);
        
        UInt16 model = 0;
        UInt8 ldn = 0;
        const char* vendor = "";
        
        switch (id) {
            case W83627DHG:
            case W83627UHG:
            case W83627DHGP:
            case W83627EHF:
            case W83627HF:
            case W83627THF:
            case W83627SF:
            case W83637HF:
            case W83667HG:
            case W83667HGB:
            case W83687THF:
            case W83697HF:
            case W83697SF:
                model = id;
                ldn = kWinbondHardwareMonitorLDN;
                vendor = "Winbond";
                break;
                
                // Fintek
            case F71858:
                model = id;
                ldn = kF71858HardwareMonitorLDN;
                vendor = "Fintek";
                break;
                
            case F71862:
            case F71869:
            case F71869A:
            case F71882:
            case F71889AD:
            case F71889ED:
            case F71889F:
            case F71808:
                model = id;
                ldn = kFintekITEHardwareMonitorLDN;
                vendor = "Fintek";
                break;
                
                // Nuvoton
            case NCT6771F:
            case NCT6776F:
                model = id;
                ldn = kWinbondHardwareMonitorLDN;
                vendor = "Nuvoton";
                break;
        }
        
        UInt16 address = 0;
        UInt16 verify = 0;
        
        if (model != 0 && ldn != 0) {
            
            superio_select_logical_device(port, ldn);
            
            address = superio_listen_port_word(port, kSuperIOBaseAddressRegister);
            
            IOSleep(50);
            
            verify = superio_listen_port_word(port, kSuperIOBaseAddressRegister);
            
            winbond_family_exit(port);
            
            if (address != verify)
                continue;
            
            // some Fintek chips have address register offset 0x05 added already
            if ((address & 0x07) == 0x05)
                address &= 0xFFF8;
            
            if (address < 0x100 || (address & 0xF007) != 0)
                continue;
        }
        else {
            winbond_family_exit(port);
            
            IOSleep(50);
            
            // IT87XX can enter only on port 0x2E
            if (port == 0x2E) {
                ite_family_enter(port);
                
                id = superio_listen_port_word(port, kSuperIOChipIDRegister);
                
                ldn = 0;
                vendor = "";
                
                switch (id) {
                    case IT8512F:
                    case IT8712F:
                    case IT8716F:
                    case IT8718F:
                    case IT8720F:
                    case IT8721F:
                    case IT8726F:
                    case IT8728F:
                    case IT8752F:
                    case IT8772E:
                        model = id;
                        ldn = kFintekITEHardwareMonitorLDN;
                        vendor = "ITE";
                        break;
                }
                
                if (model != 0 && ldn != 0) {
                    superio_select_logical_device(port, ldn);
                    
                    address = superio_listen_port_word(port, kSuperIOBaseAddressRegister);
                    
                    IOSleep(50);
                    
                    verify = superio_listen_port_word(port, kSuperIOBaseAddressRegister);
                    
                    ite_family_exit(port);
                    
                    if (address != verify || address < 0x100 || (address & 0xF007) != 0)
                        continue;
                }
                else {
                    ite_family_exit(port);
                    continue;
                }
            }
        }
        
        if (model == 0) 
            return false;
        
        IOLog("%s: found %s %s on port=0x%x address=0x%x\n", getName(), vendor, superio_get_model_name(model), port, address);
        
        char string[128];
        
        snprintf(string, sizeof(string), "%s,%s", vendor, superio_get_model_name(model));
        
        setName(string);
        
        //setProperty("name", &string, (UInt32)strlen(string) + 1);
        
        setProperty("address", address, 16);
        setProperty("port", port, 8);
        setProperty("model", model, 16);

        setProperty("model-name", superio_get_model_name(model));
        setProperty("vendor-name", vendor);
        
        registerService();
        
        break;
    }
    
    return true;
}

void SuperIO::stop(IOService *provider)
{
    super::stop(provider);
}

void SuperIO::free()
{
    super::free();
}