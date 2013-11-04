//
//  FakeSMCUserClient.cpp
//  HWSensors
//
//  Created by Kozlek on 02/11/13.
//
//

#include "FakeSMCUserClient.h"
#include "FakeSMCDefinitions.h"
#include "FakeSMC.h"
#include "FakeSMCKey.h"
#include "smc.h"

#include <IOKit/IOLib.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

static IORecursiveLock *gClientSyncLock = 0;

#define SYNCLOCK        if (!gClientSyncLock) gClientSyncLock = IORecursiveLockAlloc(); IORecursiveLockLock(gClientSyncLock)
#define SYNCUNLOCK      IORecursiveLockUnlock(gClientSyncLock)

#define super IOUserClient
OSDefineMetaClassAndStructors(FakeSMCUserClient, IOUserClient);

bool FakeSMCUserClient::start(IOService* provider)
{
    if (!super::start(provider))
        return false;

    if (!(storageProvider = OSDynamicCast(FakeSMC, provider))) {
        HWSensorsFatalLog("provider must be FakeSMC class service!");
        return false;
    }

    return true;
}

void FakeSMCUserClient::stop(IOService* provider)
{

    super::stop(provider);
}

bool FakeSMCUserClient::initWithTask(task_t owningTask, void* securityID, UInt32 type, OSDictionary* properties)
{
	if (super::initWithTask(owningTask, securityID, type, properties)) {
        storageProvider = NULL;
        return true;
	}

    HWSensorsFatalLog("super failed to initialize with task!");
	
    return false;
}

IOReturn FakeSMCUserClient::clientClose(void)
{
	if( !isInactive())
        terminate();

    return kIOReturnSuccess;
}

IOReturn FakeSMCUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments* arguments, IOExternalMethodDispatch * dispatch, OSObject * target, void * reference )
{
	IOReturn result = kIOReturnError;

	if (storageProvider == NULL || isInactive()) {
		result = kIOReturnNotAttached;
	}
	else if (!storageProvider->isOpen(this)) {
		result = kIOReturnNotOpen;
	}

    SYNCLOCK;

    switch (selector) {
        case KERNEL_INDEX_SMC: {

            SMCKeyData_t *input = (SMCKeyData_t*)arguments->structureInput;
            SMCKeyData_t *output = (SMCKeyData_t*)arguments->structureOutput;

            switch (input->data8) {
                case SMC_CMD_READ_INDEX: {
                    FakeSMCKey *key = storageProvider->getKey(input->data32);
                    output->key = _strtoul(key->getKey(), 4, 16);
                    result = kIOReturnSuccess;
                    break;
                }

                case SMC_CMD_READ_KEYINFO: {
                    char name[5];

                    _ultostr(name, input->key);

                    FakeSMCKey *key = storageProvider->getKey(name);

                    if (key) {

                        output->keyInfo.dataSize = key->getSize();
                        output->keyInfo.dataType = _strtoul(key->getType(), 4, 16);

                        result = kIOReturnSuccess;
                    }
                    else result = kIOReturnNotFound;

                    break;
                }

                case SMC_CMD_READ_BYTES: {
                    char name[5];

                    _ultostr(name, input->key);

                    FakeSMCKey *key = storageProvider->getKey(name);

                    if (key) {

                        memcpy(output->bytes, key->getValue(), key->getSize());

                        result = kIOReturnSuccess;
                    }
                    else result = kIOReturnNotFound;

                    break;
                }

                default:
                    result = kIOReturnBadArgument;
                    break;
            }

            break;
        }

        default:
            result = kIOReturnBadArgument;
            break;
    }

    SYNCUNLOCK;

    return result;
}
