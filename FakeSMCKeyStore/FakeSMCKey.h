/*
 *  FakeSMCKey.h
 *  FakeSMC
 *
 *  Created by kozlek on 03/10/10.
 *
 */

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

#ifndef _FAKESMCKEY_H
#define _FAKESMCKEY_H

#include <IOKit/IOService.h>

inline void copySymbol(const char *from, char* to)
{
    // Made the key name valid (4 char long): add trailing spaces if needed
    //bzero(to, 5);
    snprintf(to, 5, "%-4s", from);
}

class FakeSMCKeyHandler;

class FakeSMCKey : public OSObject
{
    OSDeclareDefaultStructors(FakeSMCKey)
    
private:
    char *              key;
    char *              type;
	UInt8               size;
	void *              value;
	FakeSMCKeyHandler * handler;

    double              lastValueRead;
    double              lastValueWrote;
	
public:
	static FakeSMCKey   *withValue(const char *aKey, const char *aType, const unsigned char aSize, const void *aValue);
	static FakeSMCKey   *withHandler(const char *aKey, const char *aType, const unsigned char aSize, FakeSMCKeyHandler *aHandler);
    
    // Not for general use. Use withHandler or withValue instance creation method
	virtual bool        init(const char * aKey, const char * aType, const unsigned char aSize, const void *aValue, FakeSMCKeyHandler *aHandler = 0);
	
	virtual void        free();
	
    const char          *getName(); // this is used by logging functions
    
	const char          *getKey();
	const char          *getType();
	const UInt8         getSize() const;
	const void          *getValue();
    FakeSMCKeyHandler   *getHandler();
	
    bool                setType(const char *aType);
    bool                setSize(UInt8 aSize);
	bool                setValueFromBuffer(const void *aBuffer, UInt8 aSize);
	bool                setHandler(FakeSMCKeyHandler *aHandler);
	
	bool                isEqualTo(const char *aKey);
	bool                isEqualTo(FakeSMCKey *aKey);
	bool                isEqualTo(const OSMetaClassBase *anObject);
};


#endif