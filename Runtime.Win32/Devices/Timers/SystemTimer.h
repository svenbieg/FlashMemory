//===============
// SystemTimer.h
//===============

#pragma once


//=======
// Using
//=======

#include "Event.h"


//======================
// Forward-Declarations
//======================

namespace Concurrency
{
class Task;
}


//===========
// Namespace
//===========

namespace Devices {
	namespace Timers {


//==============
// System-Timer
//==============

class SystemTimer: public Object
{
public:
	// Using
	using Task=Concurrency::Task;

	// Con-/Destructors
	~SystemTimer();

	// Common
	static Handle<SystemTimer> Get();
	static UINT64 GetTickCount();
	static UINT64 Microseconds();
	Event<SystemTimer> Triggered;

private:
	// Con-/Destructors
	SystemTimer();

	// Common
	VOID TaskProc();
	Handle<Task> m_Task;
	static SystemTimer* s_Current;
};

}}