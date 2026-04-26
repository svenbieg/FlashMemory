//===================
// CriticalSection.h
//===================

#pragma once


//=======
// Using
//=======

#include "TypeHelper.h"


//===========
// Namespace
//===========

namespace Concurrency {


//==================
// Critical Section
//==================

class CriticalSection
{
public:
	// Con-/Destructors
	CriticalSection();

	// Common
	VOID Lock();
	BOOL TryLock();
	VOID Unlock();

private:
	// Common
	CRITICAL_SECTION m_CriticalSection;
};

}