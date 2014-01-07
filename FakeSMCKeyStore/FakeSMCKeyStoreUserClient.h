//
//  FakeSMCKeyStoreUserClient.h
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

#ifndef __HWSensors__FakeSMCKeyStoreUserClient__
#define __HWSensors__FakeSMCKeyStoreUserClient__

#include <IOKit/IOUserClient.h>

class FakeSMCKeyStore;

class FakeSMCKeyStoreUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(FakeSMCKeyStoreUserClient);

private:
	FakeSMCKeyStore *keyStore;

public:
	/* IOService overrides */
	virtual bool start(IOService* provider);
	virtual void stop(IOService* provider);

	/* IOUserClient overrides */
	virtual bool initWithTask(task_t task, void* securityID, UInt32 type,  OSDictionary* properties);
	virtual IOReturn clientClose(void);
	virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments* arguments,
									IOExternalMethodDispatch* dispatch, OSObject* target, void* reference);
};

#endif /* defined(__HWSensors__FakeSMCKeyStoreUserClient__) */
