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
    
    Device (_SB.PROB) // ACPIProbe virtual device (c) TimeWalker
    {
        Name (_HID, EisaId ("PRB0000")) // Expose PLLD to IORegistry
        Name (_CID, "acpi-probe") // device compatible name allows ACPIProbe matching

        /* Define settings for ACPI method polling */
       
        Name (LIST, Package (0x03) // profiles list
        {
           "PFL0",
           "PFL1",
           "PFL2"
        })

        Name (ACTV, "PFL2") // startup profile

        Name (PFL0, Package () // audible
        {
           "Audible",
           0x03E8, // polling interval
           Zero,   // polling timeout
           Zero,   // disable console logging
           "TAVG", // method 1
           "FAUD"  // method 2
        })

        Name (PFL1, Package () // passive
        {
           "Passive",
           0x03E8,
           Zero,
           Zero,
           "TAVG",
           "FPAS"
        })
        Name (PFL2, Package () // automatic
        {
           "Automatic",
           0x03E8,
           0x0BB8, // reset and stop polling methods after 3 sec
           Zero,
           "FRST"
        })

        Method (FRST, 0, NotSerialized) // Test method, defined in "Automatic" profile
        {
            Store (0x18894, Local0)
            Return (Local0)
        }
    }
}

