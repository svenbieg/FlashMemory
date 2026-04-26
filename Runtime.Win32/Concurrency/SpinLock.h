//============
// SpinLock.h
//============

#pragma once


//=======
// Using
//=======

#include "Concurrency/CriticalSection.h"
#include "Concurrency/ScopedLock.h"


//===========
// Namespace
//===========

namespace Concurrency {


//===========
// Spin-Lock
//===========

class SpinLock: public ScopedLock
{
public:
	// Con-/Destructors
	SpinLock(CriticalSection& CriticalSection): m_CriticalSection(&CriticalSection)
		{
		m_CriticalSection->Lock();
		}
	~SpinLock()
		{
		if(m_CriticalSection)
			m_CriticalSection->Unlock();
		}

	// Common
	inline VOID Lock()override { m_CriticalSection->Lock(); }
	inline VOID Release()override { m_CriticalSection=nullptr; }
	inline BOOL TryLock()override { return m_CriticalSection->TryLock(); }
	inline VOID Unlock()override { m_CriticalSection->Unlock(); }

private:
	// Common
	CriticalSection* m_CriticalSection;
};

}