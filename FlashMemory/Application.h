//===============
// Application.h
//===============

#pragma once


//=======
// Using
//=======

#include "Concurrency/TaskMonitor.h"
#include "Devices/System/StatusLed.h"
#include "Devices/Timers/SystemTimer.h"
#include "Storage/Database/Database.h"


//===========
// Namespace
//===========

namespace FlashMemory {


//=============
// Application
//=============

class Application: public Object
{
public:
	// Friends
	friend Object;

	// Using
	using Database=Storage::Database::Database;
	using Page=Storage::Page;
	using StatusLed=Devices::System::StatusLed;
	using SystemTimer=Devices::Timers::SystemTimer;
	using TaskMonitor=Concurrency::TaskMonitor;
	using Volume=Storage::Volume;

	// Con-/Destructors
	static inline Handle<Application> Create() { return Object::Create<Application>(); }

	// Common
	VOID Run();

private:
	// Con-/Destructors
	Application();

	// Common
	VOID PrintBuffer(BYTE const* Buffer, UINT Size);
	VOID PrintPage(Page* Page);
	Handle<Page> ReadPage(UINT Block, WORD Page);
	VOID TaskInfo();
	Handle<Database> m_Database;
	Handle<StatusLed> m_StatusLed;
	Handle<SystemTimer> m_SystemTimer;
	Handle<TaskMonitor> m_TaskMonitor;
	Handle<Volume> m_Volume;
};

}