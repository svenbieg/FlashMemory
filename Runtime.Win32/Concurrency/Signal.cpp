//============
// Signal.cpp
//============

#include "Signal.h"


//=======
// Using
//=======

#include "Concurrency/Task.h"


//===========
// Namespace
//===========

namespace Concurrency {


//========
// Common
//========

BOOL Signal::Wait()
{
Task::ThrowIfMain();
Mutex mutex;
WriteLock lock(mutex);
wait(lock);
return true;
}

BOOL Signal::Wait(ScopedLock& lock)
{
Task::ThrowIfMain();
auto write_lock=dynamic_cast<WriteLock*>(&lock);
if(write_lock)
	{
	wait(*write_lock);
	return true;
	}
auto read_lock=dynamic_cast<ReadLock*>(&lock);
if(read_lock)
	{
	wait(*read_lock);
	return true;
	}
throw NotImplementedException();
}

BOOL Signal::Wait(ScopedLock& lock, UINT timeout)
{
Task::ThrowIfMain();
auto write_lock=dynamic_cast<WriteLock*>(&lock);
if(write_lock)
	{
	auto status=wait_for(*write_lock, std::chrono::milliseconds(timeout));
	return (status==std::cv_status::no_timeout);
	}
auto read_lock=dynamic_cast<ReadLock*>(&lock);
if(read_lock)
	{
	auto status=wait_for(*read_lock, std::chrono::milliseconds(timeout));
	return (status==std::cv_status::no_timeout);
	}
throw NotImplementedException();
}


//================
// Common Private
//================

BOOL Signal::WaitInternal(ScopedLock& lock)
{
auto write_lock=dynamic_cast<WriteLock*>(&lock);
if(write_lock)
	{
	wait(*write_lock);
	return true;
	}
throw NotImplementedException();
}

}