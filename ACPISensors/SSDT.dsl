DefinitionBlock ("SSDT.aml", "SSDT", 1, "APPLE ", "DefMon", 0x00003000)
{
    Device (_SB.MNTR)
    {
        Name (_HID, "MONITOR")
        
    Name (KLVN, Zero) // Not use Kelvins (Celsius instead)
        
    Name (TEMP, Package (0x04) // temperature description table
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
}

