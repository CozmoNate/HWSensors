//
//  ACPIProbeUserClient.cpp
//  HWSensors
//
//  Created by Kozlek on 17/02/14.
//
//

//  The MIT License (MIT)
//
//  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "ACPIProbeUserClient.h"

#include "ACPIProbe.h"
#include "ACPIProbeArgument.h"

#include <IOKit/IOLib.h>

static IORecursiveLock *gClientSyncLock = 0;

#define SYNCLOCK        if (!gClientSyncLock) gClientSyncLock = IORecursiveLockAlloc(); IORecursiveLockLock(gClientSyncLock)
#define SYNCUNLOCK      IORecursiveLockUnlock(gClientSyncLock)

#define super IOUserClient
OSDefineMetaClassAndStructors(ACPIProbeUserClient, IOUserClient);

bool ACPIProbeUserClient::start(IOService* provider)
{
    if (!super::start(provider))
        return false;

    if (!(acpiProbe = OSDynamicCast(ACPIProbe, provider))) {
        HWSensorsFatalLog("provider must be ACPIProbe class service!");
        return false;
    }

    return true;
}

void ACPIProbeUserClient::stop(IOService* provider)
{
    super::stop(provider);
}

bool ACPIProbeUserClient::initWithTask(task_t owningTask, void* securityID, UInt32 type, OSDictionary* properties)
{
	if (super::initWithTask(owningTask, securityID, type, properties)) {
        acpiProbe = NULL;
        return true;
	}

    HWSensorsFatalLog("super failed to initialize with task!");

    return false;
}

IOReturn ACPIProbeUserClient::clientClose(void)
{
	if( !isInactive())
        terminate();

    return kIOReturnSuccess;
}

IOReturn ACPIProbeUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments* arguments, IOExternalMethodDispatch * dispatch, OSObject * target, void * reference )
{
	IOReturn result = kIOReturnError;

	if (acpiProbe == NULL || isInactive()) {
		result = kIOReturnNotAttached;
	}
	else if (!acpiProbe->isOpen(this)) {
		result = kIOReturnNotOpen;
	}

    SYNCLOCK;

    switch (selector) {
        case ACPIPRB_CMD_GET_PROFILE_COUNT:{

            ACPIProbeArgument *output = (ACPIProbeArgument *)arguments->structureOutput;

            output->data_uint32 = acpiProbe->getProfileCount();
            output->result = kIOReturnSuccess;

            result = kIOReturnSuccess;
            break;
        }

        case ACPIPRB_CMD_GET_PROFILE_AT_INDEX: {

            ACPIProbeArgument *input = (ACPIProbeArgument *)arguments->structureInput;
            ACPIProbeArgument *output = (ACPIProbeArgument *)arguments->structureOutput;

            if (input->data_uint32 < acpiProbe->getProfileCount()) {
                if (ACPIProbeProfile *profile = acpiProbe->getProfile(input->data_uint32)) {
                    snprintf(output->data_string, 32, "%s", profile->name);
                    output->result = kIOReturnSuccess;
                }
                else {
                    output->result = kIOReturnInternalError;
                }
            }
            else {
                output->result = kIOReturnBadArgument;
            }

            result = kIOReturnSuccess;
            break;
        }

        case ACPIPRB_CMD_GET_ACTIVE_PROFILE: {

            ACPIProbeArgument *output = (ACPIProbeArgument *)arguments->structureOutput;

            if (ACPIProbeProfile *profile = acpiProbe->getActiveProfile()) {
                snprintf(output->data_string, 32, "%s", profile->name);
                output->result = kIOReturnSuccess;
            }
            else {
                bzero(output->data_string, 32);
                output->result = kIOReturnSuccess;
            }

            result = kIOReturnSuccess;
            break;
        }

        case ACPIPRB_CMD_SET_ACTIVE_PROFILE: {

            ACPIProbeArgument *input = (ACPIProbeArgument *)arguments->structureInput;
            ACPIProbeArgument *output = (ACPIProbeArgument *)arguments->structureOutput;

            output->result = acpiProbe->activateProfile(input->data_string);

            result = kIOReturnSuccess;
            break;
        }

        default:
            result = kIOReturnBadArgument;
            break;
    }
    
    SYNCUNLOCK;
    
    return result;
}
