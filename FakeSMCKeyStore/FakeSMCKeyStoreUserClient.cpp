//
//  FakeSMCKeyStoreUserClient.cpp
//  HWSensors
//
//  Created by Kozlek on 02/11/13.
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

#include "FakeSMCKeyStoreUserClient.h"

#include "FakeSMCDefinitions.h"
#include "FakeSMCKeyStore.h"
#include "FakeSMCKey.h"
#include "smc.h"

#include <IOKit/IOLib.h>

static IORecursiveLock *gClientSyncLock = 0;

#define SYNCLOCK        if (!gClientSyncLock) gClientSyncLock = IORecursiveLockAlloc(); IORecursiveLockLock(gClientSyncLock)
#define SYNCUNLOCK      IORecursiveLockUnlock(gClientSyncLock)

#define super IOUserClient
OSDefineMetaClassAndStructors(FakeSMCKeyStoreUserClient, IOUserClient);

bool FakeSMCKeyStoreUserClient::start(IOService* provider)
{
    if (!super::start(provider))
        return false;

    if (!(keyStore = OSDynamicCast(FakeSMCKeyStore, provider))) {
        HWSensorsFatalLog("provider must be FakeSMCKeyStore class service!");
        return false;
    }

    return true;
}

void FakeSMCKeyStoreUserClient::stop(IOService* provider)
{
    super::stop(provider);
}

bool FakeSMCKeyStoreUserClient::initWithTask(task_t owningTask, void* securityID, UInt32 type, OSDictionary* properties)
{
	if (super::initWithTask(owningTask, securityID, type, properties)) {
        keyStore = NULL;
        return true;
	}

    HWSensorsFatalLog("super failed to initialize with task!");
	
    return false;
}

IOReturn FakeSMCKeyStoreUserClient::clientClose(void)
{
	if( !isInactive())
        terminate();

    return kIOReturnSuccess;
}

IOReturn FakeSMCKeyStoreUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments* arguments, IOExternalMethodDispatch * dispatch, OSObject * target, void * reference )
{
	IOReturn result = kIOReturnError;

	if (keyStore == NULL || isInactive()) {
		result = kIOReturnNotAttached;
	}
	else if (!keyStore->isOpen(this)) {
		result = kIOReturnNotOpen;
	}

    SYNCLOCK;

    switch (selector) {
        case KERNEL_INDEX_SMC: {

            SMCKeyData_t *input = (SMCKeyData_t*)arguments->structureInput;
            SMCKeyData_t *output = (SMCKeyData_t*)arguments->structureOutput;

            switch (input->data8) {
                case SMC_CMD_READ_INDEX: {
                    FakeSMCKey *key = keyStore->getKey(input->data32);
                    output->key = _strtoul(key->getKey(), 4, 16);
                    result = kIOReturnSuccess;
                    break;
                }

                case SMC_CMD_READ_KEYINFO: {
                    char name[5];

                    _ultostr(name, input->key);

                    FakeSMCKey *key = keyStore->getKey(name);

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

                    FakeSMCKey *key = keyStore->getKey(name);

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
