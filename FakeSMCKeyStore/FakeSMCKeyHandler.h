//
//  FakeSMCKeyHandler.h
//  HWSensors
//
//  Created by Kozlek on 06/11/13.
//
//

#ifndef __HWSensors__FakeSMCKeyHandler__
#define __HWSensors__FakeSMCKeyHandler__

#include <IOKit/IOService.h>

class FakeSMCKeyHandler : public IOService {

	OSDeclareAbstractStructors(FakeSMCKeyHandler)

public:
    UInt32              getProbeScore();
    virtual IOReturn    getValueCallback(const char *key, const char *type, const UInt8 size, void *buffer);
    virtual IOReturn    setValueForKey(const char *key, const char *type, const UInt8 size, const void *value);

};

#endif /* defined(__HWSensors__FakeSMCKeyHandler__) */
