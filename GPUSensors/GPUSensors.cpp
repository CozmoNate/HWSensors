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

IOService* GPUSensors::probe(IOService *provider, SInt32 *score)
{
    if (!provider || super::probe(provider, score) != this)
        return 0;

    OSDictionary *matching = serviceMatching("IOAccelerator");
    
    bool acceleratorFound = false;
    
    if (matching) {
        
        if (OSIterator *iterator = getMatchingServices(matching)) {
            while (IOService *child = (IOService*)iterator->getNextObject()) {
                if (provider->isChild(child, gIOServicePlane)) {
                    acceleratorFound = true;
                    break;
                }
            }
            
            OSSafeRelease(iterator);
        }
        
        OSSafeRelease(matching);
    }
    
    return acceleratorFound ? this : 0;
}