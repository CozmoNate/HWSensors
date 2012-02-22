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
    return swap_value((UInt16)(tmp & 0xfffc));
}

inline UInt16 encode_fp4c(UInt16 value)
{    
    UInt32 tmp = value;
    tmp = (tmp << 12) / 1000;
    return swap_value((UInt16)(tmp & 0xfffc));
}

inline UInt16 encode_fpe2(UInt16 value)
{
	return swap_value(value << 2);
}

inline UInt16 decode_fpe2(UInt16 value)
{
	return (swap_value(value) >> 2);
}