/*
 *  FakeSMCUtils.h
 *  HWSensors
 *
 *  Created by mozo on 05.04.11.
 *  Copyright 2011 mozodojo. All rights reserved.
 *
 */

inline UInt16 swap_value(UInt16 value)
{
	return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
}

inline UInt16 encode_fp2e(UInt16 value)
{   
    UInt32 tmp = value;
    tmp = (tmp << 14) / 1000;
    return swap_value((UInt16)(tmp & 0xffff));
}

inline float decode_fp2e(UInt16 encoded)
{
    encoded = swap_value(encoded);
    
    return (float)encoded / (float)(1 << 12);
};

inline UInt16 encode_fp4c(UInt16 value)
{    
    UInt32 tmp = value;
    tmp = (tmp << 12) / 1000;
    return swap_value((UInt16)(tmp & 0xffff));
}

inline float decode_fp4c(UInt16 encoded)
{
    encoded = swap_value(encoded);
    
    return (float)encoded / (float)(1 << 14);
};

inline UInt16 encode_fpe2(UInt16 value)
{
	return swap_value(value << 2);
}

inline UInt16 decode_fpe2(UInt16 value)
{
	return (swap_value(value) >> 2);
}

inline UInt8 get_index(char c)
{
	return c >= 'a' ? c - 87 : c >= 'A' ? c - 55 : c - 48;
};

inline UInt16 encode_float(const char *type, long value)
{
    UInt8 i = 0, f = 0;
    
    if (type[0] == 's' || type[0] == 'f') {
        if (type[1] == 'p') {
            i = get_index(type[2]);
            f = get_index(type[3]);
            
            if (i + f != 16 ) 
                return value;
        }
        else return value;
        
        return swap_value(((value << f) / 1000) & 0xffff);
    }
    
    return value; // leave as is
}

inline UInt16 encode_long(const char *type, long value)
{
    UInt8 i = 0, f = 0;
    
    if (type[0] == 's' || type[0] == 'f') {
        if (type[1] == 'p') {
            i = get_index(type[2]);
            f = get_index(type[3]);
            
            if (i + f != 16 ) 
                return value;
        }
        else return value;
        
        return swap_value((value << f) & 0xffff);
    }
    
    return value; // leave as is
}