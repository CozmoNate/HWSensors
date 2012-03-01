//
//  FakeSMCValueEncoder.h
//  HWSensors
//
//  Created by kozlek on 28/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#ifndef HWSensors_FakeSMCValueEncoder_h
#define HWSensors_FakeSMCValueEncoder_h

inline uint32_t key_to_int(const char *name)
{
    return *((uint32_t*)name);
}

inline UInt8 get_index(char c)
{
	return c >= 'a' ? c - 87 : c >= 'A' ? c - 55 : c - 48;
};

inline UInt16 encode_16bit_fractional(const char *type, float value)
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
        
        UInt64 mult = value * 1000;
        
        return OSSwapHostToBigInt16(((mult << f) / 1000) & 0xffff);
    }
    
    return value; // leave as is
}

inline float decode_16bit_fractional(const char* type, UInt16 encoded)
{
    UInt8 i = 0, f = 0;
    
    encoded = OSSwapBigToHostInt16(encoded);
    
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

#endif
