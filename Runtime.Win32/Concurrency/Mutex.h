//=========
// Mutex.h
//=========

#pragma once


//=======
// Using
//=======

#include "TypeHelper.h"
#include <mutex>


//===========
// Namespace
//===========

namespace Concurrency {


//======================
// Forward-Declarations
//======================

class ReadLock;
class WriteLock;


//=============
// Access-Mode
//=============

enum class AccessMode
{
ReadOnly
};


//=======
// Mutex
//=======

class Mutex: protected std::mutex
{
public:
	// Friends
	friend ReadLock;
	friend WriteLock;

	// Con-/Destructors
	Mutex() {}

	// Common
	inline VOID Lock() { lock(); }
	inline VOID Lock(AccessMode) { lock(); }
	inline BOOL TryLock() { return try_lock(); }
	inline VOID Unlock() { unlock(); }
	inline VOID Unlock(AccessMode) { unlock(); }
};

}