//
//  FakeSMCKeykeyStore.h
//  HWSensors
//
//  Created by Kozlek on 04/11/13.
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
