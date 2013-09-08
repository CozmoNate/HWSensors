DefinitionBlock ("SSDT.aml", "SSDT", 1, "APPLE ", "DefMon", 0x00003000)
{
    Device (_SB.MON0) // ACPISensors virtual device
    {
        Name (_HID, EisaId ("PNP0C02")) // Expose MON0 to IORegistry
        Name (_CID, "MON00000") // Allows ACPISensors matching
        
        Name (KLVN, Zero) // Not use Kelvins (Celsius instead)
            
        Name (TEMP, Package (0x04) // temperature description table, key names are gotten from FakeSMCPlugin.h
        {
            "Thermal Zone", 
            "TMP0", 
            "Thermal Zone", 
            "TMP1"
        })
        Method (TMP0, 0, NotSerialized) // Test temperature method, returns 100
        {
            Store (0x64, Local0)
            Return (Local0)
        }
        Method (TMP1, 0, NotSerialized) // Test temperature method, returns 101
        {
            Store (0x65, Local0)
            Return (Local0)
        }
            
        Name (VOLT, Package (0x02) // voltage description table
        {
            "Power Supply", 
            "VLT0"
        })
        Method (VLT0, 0, NotSerialized) // Test voltage method, returns 8888 = 8.888V
        {
            Store (0x22B8, Local0)
            Return (Local0)
        }

        Name (TACH, Package (0x02) // tachometer description table
        {
            "Test Fan 0", 
            "FAN0"
        })
        Method (FAN0, 0, NotSerialized) // Test fan method, returns 50
        {
            Store (0x32, Local0)
            Return (Local0)
        }
    }
    
    Device (_SB.PLLD) // ACPIPoller virtual device (c) TimeWalker
    {
        Name (_HID, EisaId ("PNP0C02")) // Expose PLLD to IORegistry
        Name (_CID, EisaId ("PLL0000")) // device compatible name allows ACPIPoller matching

        /* Define settings for ACPI method polling */
       
        Name (INVL, 0x3E8)          // Set Polling interval 1 sec
        Name (TOUT, Zero)           // Set Polling timeout  0 sec (continuous polling)
        Name (LOGG, One)            // Enable Console logging of values returned by methods
        Name (LIST, Package (0x02)  // Define methods to poll
        {
            "TST0",
            "TST1"
        })
        
        Method (TST0, 0, NotSerialized) // Test method returns 100500
        {
            Store (0x18894, Local0)
            Return (Local0)
        }
    }
}

