//=====================
// CriticalSection.cpp
//=====================

#include "CriticalSection.h"


//===========
// Namespace
//===========

namespace Concurrency {


//==================
// Con-/Destructors
//==================

CriticalSection::CriticalSection()
{
InitializeCriticalSection(&m_CriticalSection);
}


//========
// Common
//========

VOID CriticalSection::Lock()
{
EnterCriticalSection(&m_CriticalSection);
}

BOOL CriticalSection::TryLock()
{
return TryEnterCriticalSection(&m_CriticalSection);
}

VOID CriticalSection::Unlock()
{
LeaveCriticalSection(&m_CriticalSection);
}

}