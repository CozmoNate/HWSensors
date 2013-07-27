//
//  GPUSensors.cpp
//  HWSensors
//
//  Created by Kozlek on 27/07/13.
//
//

#include "GPUSensors.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(GPUSensors, FakeSMCPlugin)

bool GPUSensors::start(IOService *provider)
{
    if (!provider || !super::start(provider))
        return false;

    bool acceleratorFound = false;
    
    if (OSDictionary *matching = serviceMatching("IOAccelerator")) {
        if (OSIterator *iterator = getMatchingServices(matching)) {
            while (IOService *service = (IOService*)iterator->getNextObject()) {
                if (IORegistryEntry *parent = service->getParentEntry(gIOServicePlane)) {
                    acceleratorFound = parent == provider;
                }
            }
            
            OSSafeRelease(iterator);
        }
        
        OSSafeRelease(matching);
    }
    
    return acceleratorFound;
}