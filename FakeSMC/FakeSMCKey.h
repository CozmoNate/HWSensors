/*
 *  FakeSMCKey.h
 *  FakeSMC
 *
 *  Created by mozo on 03/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _FAKESMCKEY_H
#define _FAKESMCKEY_H

#include <IOKit/IOService.h>
#include <kern/clock.h>

inline void copySymbol(const char *from, char* to)
{
	bzero(to, 5);
	
	UInt8 len = (UInt8)strlen(from);
	
	bcopy(from, to, len > 4 ? 4 : len);
}

class FakeSMCKey : public OSObject
{
    OSDeclareDefaultStructors(FakeSMCKey)
    
private:
    mach_timespec_t lastUpdated;
	
protected:
    char *			name;
    char *			type;
	UInt8           size;
	void *			value;
	IOService *		handler;
	
public:
	static FakeSMCKey *withValue(const char *aName, const char *aType, unsigned char aSize, const void *aValue);
	static FakeSMCKey *withHandler(const char *aName, const char *aType, unsigned char aSize, IOService *aHandler);
	
	// Not for general use. Use withCallback or withValue instance creation method
	virtual bool init(const char * aName, const char * aType, unsigned char aSize, const void *aValue, IOService *aHandler = 0);
	
	virtual void free();
	
	const char *getName();
	const char *getType();
	UInt8 getSize() const;
	const void *getValue();
    const IOService *getHandler();
	
	bool setValueFromBuffer(const void *aBuffer, UInt8 aSize);
	bool setHandler(IOService *aHandler);
	
	bool isEqualTo(const char *aKey);
	bool isEqualTo(FakeSMCKey *aKey);
	bool isEqualTo(const OSMetaClassBase *anObject);
};


#endif