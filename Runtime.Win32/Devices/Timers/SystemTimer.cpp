//=================
// SystemTimer.cpp
//=================

#include "SystemTimer.h"


//=======
// Using
//=======

#include "Concurrency/Task.h"

using namespace Concurrency;


//===========
// Namespace
//===========

namespace Devices {
	namespace Timers {


//==================
// Con-/Destructors
//==================

SystemTimer::~SystemTimer()
{
if(m_Task)
	{
	m_Task->Cancel();
	m_Task=nullptr;
	}
s_Current=nullptr;
}


//========
// Common
//========

Handle<SystemTimer> SystemTimer::Get()
{
if(!s_Current)
	s_Current=new SystemTimer();
return s_Current;
}

UINT64 SystemTimer::GetTickCount()
{
return ::GetTickCount64();
}

UINT64 SystemTimer::Microseconds()
{
LARGE_INTEGER ticks;
QueryPerformanceFrequency(&ticks);
ticks.QuadPart/=1000;
LARGE_INTEGER time;
QueryPerformanceCounter(&time);
return time.QuadPart/ticks.QuadPart;
}


//==========================
// Con-/Destructors Private
//==========================

SystemTimer::SystemTimer()
{
s_Current=this;
m_Task=Task::Create(this, &SystemTimer::TaskProc);
}


//================
// Common Private
//================

VOID SystemTimer::TaskProc()
{
auto task=Task::Get();
while(!task->Cancelled)
	{
	DispatchedQueue::Append(this, [this]() { Triggered(this); });
	Sleep(10);
	}
}

SystemTimer* SystemTimer::s_Current=nullptr;

}}