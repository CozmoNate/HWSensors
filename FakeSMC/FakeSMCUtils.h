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

inline float decode_to_float(const char* type, UInt16 encoded)
{
    UInt8 i = 0, f = 0;
    
    encoded = swap_value(encoded);
    
    if (type[0] == 's' || type[0] == 'f') {
        if (type[1] == 'p') {
            i = get_index(type[2]);
            f = get_index(type[3]);
            
            if (i + f != (type[0] == 's' ? 15 : 16) ) 
                return encoded;
        }
        else return encoded;
        
        float value = (float)encoded / (float)(0x1 << f);
        
        return value * (type[0] == 's' && encoded & 0x8000 ? -1 : 1);
    }
    
    return encoded;
}

inline UInt16 decode_to_long(const char* type, UInt16 encoded)
{
    UInt8 i = 0, f = 0;
    
    encoded = swap_value(encoded);
    
    if (type[0] == 's' || type[0] == 'f') {
        if (type[1] == 'p') {
            i = get_index(type[2]);
            f = get_index(type[3]);
            
            if (i + f != (type[0] == 's' ? 15 : 16) ) 
                return encoded;
        }
        else return encoded;
        
        float value = (float)encoded / (float)(0x1 << f);
        
        return (UInt16)(value * 1000) | (type[0] == 's' ? encoded & 0x8000 : 0x0);
    }
    
    return encoded;
}