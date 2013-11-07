//
//  FakeSMCKeykeyStore.h
//  HWSensors
//
//  Created by Kozlek on 04/11/13.
//
//

#ifndef __HWSensors__FakeSMCKeykeyStore__
#define __HWSensors__FakeSMCKeykeyStore__

#include <IOKit/IOService.h>

class FakeSMCKey;
class FakeSMCKeyHandler;

class FakeSMCKeyStore : public IOService
{
    OSDeclareDefaultStructors(FakeSMCKeyStore)

private:
    OSArray             *keys;
    OSDictionary        *types;

   	FakeSMCKey			*keyCounterKey;
    FakeSMCKey          *fanCounterKey;

    UInt16              vacantGPUIndex;
    UInt16              vacantFanIndex;

    bool                useNVRAM;
    bool                genericNVRAM;

public:
    FakeSMCKey          *addKeyWithValue(const char *name, const char *type, unsigned char size, const void *value);
	FakeSMCKey          *addKeyWithHandler(const char *name, const char *type, unsigned char size, FakeSMCKeyHandler *handler);
	FakeSMCKey          *getKey(const char *name);
	FakeSMCKey          *getKey(unsigned int index);
    OSArray             *getKeys(void);
	UInt32              getCount(void);

    void                updateKeyCounterKey(void);
    void                updateFanCounterKey(void);

    UInt32              addKeysFromDictionary(OSDictionary* dictionary);
    UInt32              addWellKnownTypesFromDictionary(OSDictionary* dictionary);
    void                saveKeyToNVRAM(FakeSMCKey *key);
    UInt32              loadKeysFromNVRAM();

    SInt8               takeVacantGPUIndex();
    bool                takeGPUIndex(UInt8 index);
    void                releaseGPUIndex(UInt8 index);
    SInt8               takeVacantFanIndex(void);
    void                releaseFanIndex(UInt8 index);

    virtual bool		initAndStart(IOService *provider, OSDictionary *properties = 0);

    virtual bool		init(OSDictionary *dictionary = 0);
    virtual bool		start(IOService *provider);
    virtual void        free();

    virtual IOReturn    newUserClient(task_t owningTask, void *security_id, UInt32 type, IOUserClient ** handler);

};

#endif /* defined(__HWSensors__FakeSMCKeykeyStore__) */
