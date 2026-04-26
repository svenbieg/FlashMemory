//==========
// Signal.h
//==========

#pragma once


//=======
// Using
//=======

#include "Concurrency/ScopedLock.h"
#include "Exception.h"
#include <condition_variable>


//===========
// Namespace
//===========

namespace Concurrency {


//======================
// Forward-Declarations
//======================

class DispatchedQueue;


//========
// Signal
//========

class Signal: private std::condition_variable
{
public:
	// Friends
	friend DispatchedQueue;

	// Con-/Destructors
	Signal() {}

	// Common
	inline VOID Cancel() { throw NotImplementedException(); }
	inline VOID Trigger() { notify_all(); }
	BOOL Wait();
	BOOL Wait(ScopedLock& Lock);
	BOOL Wait(ScopedLock& Lock, UINT Timeout);

private:
	// Common
	BOOL WaitInternal(ScopedLock& Lock);
};

}