//
//  FakeSMCKeyHandler.cpp
//  HWSensors
//
//  Created by Kozlek on 06/11/13.
//
//

#include "FakeSMCKeyHandler.h"

#define super IOService
OSDefineMetaClassAndAbstractStructors(FakeSMCKeyHandler, IOService)

UInt32 FakeSMCKeyHandler::getProbeScore()
{
    if (OSNumber *priority = OSDynamicCast(OSNumber, getProperty("IOProbeScore"))) {
        return priority->unsigned32BitValue();
    }

    return 0;
}

IOReturn FakeSMCKeyHandler::getValueCallback(const char *key, const char *type, const UInt8 size, void *buffer)
{
    return kIOReturnUnsupported;
}

IOReturn FakeSMCKeyHandler::setValueForKey(const char *key, const char *type, const UInt8 size, const void *value)
{
    return kIOReturnUnsupported;
}