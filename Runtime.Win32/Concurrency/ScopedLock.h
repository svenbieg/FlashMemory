//==============
// ScopedLock.h
//==============

#pragma once


//=======
// Using
//=======

#include "Concurrency/Mutex.h"


//===========
// Namespace
//===========

namespace Concurrency {


//=============
// Scoped-Lock
//=============

class ScopedLock
{
public:
	// Common
	virtual VOID Lock()=0;
	virtual VOID Release()=0;
	virtual BOOL TryLock()=0;
	virtual VOID Unlock()=0;

protected:
	// Common
	ScopedLock() {}
};

}