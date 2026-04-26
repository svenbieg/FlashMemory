//============
// ReadLock.h
//============

#pragma once


//=======
// Using
//=======

#include "Concurrency/ScopedLock.h"


//===========
// Namespace
//===========

namespace Concurrency {


//======================
// Forward-Declarations
//======================

class Signal;


//===========
// Read-Lock
//===========

class ReadLock: public ScopedLock, protected std::unique_lock<std::mutex>
{
public:
	// Friends
	friend Signal;

	// Con-/Destructors
	ReadLock(Mutex& Mutex): unique_lock(Mutex) {}

	// Common
	inline VOID Lock()override { lock(); }
	inline VOID Release()override { release(); }
	inline BOOL TryLock()override { return try_lock(); }
	inline VOID Unlock()override { unlock(); }
};

}