//
//  FakeSMCUserClient.h
//  HWSensors
//
//  Created by Kozlek on 02/11/13.
//
//

#ifndef __HWSensors__FakeSMCUserClient__
#define __HWSensors__FakeSMCUserClient__

#include <IOKit/IOUserClient.h>

class FakeSMC;

class FakeSMCUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(FakeSMCUserClient);

private:
	FakeSMC *storageProvider;

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

#endif /* defined(__HWSensors__FakeSMCUserClient__) */
