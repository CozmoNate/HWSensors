#ifndef _VIRTUALSMC_H
#define _VIRTUALSMC_H

#include <IOKit/IOService.h>


class FakeSMCKey;
class FakeSMCDevice;

class FakeSMC : public IOService
{
	OSDeclareDefaultStructors(FakeSMC)
	
private:
	FakeSMCDevice		*smcDevice;

    OSArray             *keys;
    OSDictionary        *types;

   	FakeSMCKey			*keyCounterKey;
    FakeSMCKey          *fanCounterKey;

    UInt16              vacantGPUIndex;
    UInt16              vacantFanIndex;

    bool                debug;

    bool                useNVRAM;
    bool                genericNVRAM;

public:
    FakeSMCKey          *addKeyWithValue(const char *name, const char *type, unsigned char size, const void *value);
	FakeSMCKey          *addKeyWithHandler(const char *name, const char *type, unsigned char size, IOService *handler);
	FakeSMCKey          *getKey(const char *name);
	FakeSMCKey          *getKey(unsigned int index);
    OSArray             *getKeys(void);
	UInt32              getCount(void);

    void                updateKeyCounterKey(void);
    void                updateFanCounterKey(void);

    SInt8               takeVacantGPUIndex();
    bool                takeGPUIndex(UInt8 index);
    void                releaseGPUIndex(UInt8 index);
    SInt8               getVacantFanIndex(void);
    void                releaseFanIndex(UInt8 index);

    void                saveKeyToNVRAM(FakeSMCKey *key);
    UInt32              loadKeysFromNVRAM();

    virtual IOService	*probe(IOService *provider, SInt32 *score);
    virtual bool		init(OSDictionary *dictionary = 0);
    virtual bool		start(IOService *provider);
    virtual void		stop(IOService *provider);
    virtual void		free(void);

    virtual IOReturn    newUserClient(task_t owningTask, void *security_id, UInt32 type, IOUserClient ** handler);

    virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};

#endif