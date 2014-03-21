//
//  acpiprobe.h
//  HWSensors
//
//  Created by Kozlek on 17/02/14.
//
//

#ifndef HWSensors_acpiprobe_h
#define HWSensors_acpiprobe_h

#define ACPIPRB_CMD_GET_PROFILE_COUNT       1
#define ACPIPRB_CMD_GET_PROFILE_AT_INDEX    2
#define ACPIPRB_CMD_GET_ACTIVE_PROFILE      3
#define ACPIPRB_CMD_SET_ACTIVE_PROFILE      4

typedef struct {
    char            data_string[32];
    unsigned int    data_uint32;
    IOReturn        result;
} ACPIProbeArgument;

#endif
