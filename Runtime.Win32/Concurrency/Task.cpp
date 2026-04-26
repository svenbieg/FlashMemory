//==========
// Task.cpp
//==========

#include "Task.h"


//=======
// Using
//=======

using namespace Collections;


//===========
// Namespace
//===========

namespace Concurrency {


//==================
// Con-/Destructors
//==================

Task::~Task()
{
if(m_Thread!=NULL)
	{
	CloseHandle(m_Thread);
	m_Thread=NULL;
	}
if(m_Then)
	{
	delete m_Then;
	m_Then=nullptr;
	}
s_Tasks.remove(m_Id);
}


//========
// Common
//========

VOID Task::Cancel()
{
WriteLock lock(m_Mutex);
if(m_Thread==NULL)
	return;
Cancelled=true;
}

Handle<Task> Task::Create(VOID (*proc)(), Handle<String> name, UINT stack_size)
{
Handle<Task> task=new TaskProcedure(proc, name, stack_size);
RunDeferred(task);
return task;
}

Handle<Task> Task::Get()
{
DWORD id=GetCurrentThreadId();
return s_Tasks.get(id);
}

Handle<Object> Task::GetResult()
{
Task::ThrowIfMain();
WriteLock lock(m_Mutex);
if(m_Thread==NULL)
	return Result;
if(m_Status!=Status::Pending)
	return Result;
m_Done.Wait(lock);
return Result;
}

VOID Task::Sleep(UINT ms)
{
Task::ThrowIfMain();
::Sleep(ms);
}

VOID Task::SleepMicroseconds(UINT us)
{
Task::ThrowIfMain();
}

Status Task::Wait()
{
Task::ThrowIfMain();
WriteLock lock(m_Mutex);
if(m_Thread==NULL)
	return m_Status;
if(m_Status!=Status::Pending)
	return m_Status;
m_Done.Wait(lock);
return m_Status;
}


//============================
// Con-/Destructors Protected
//============================

Task::Task(Handle<String> name, UINT stack_size):
Cancelled(false),
m_Id(0),
m_Name(name),
m_Status(Status::Pending),
m_Then(nullptr),
m_This(this),
m_Thread(NULL)
{
m_Thread=CreateThread(nullptr, stack_size, TaskProc, this, CREATE_SUSPENDED, &m_Id);
if(m_Thread==INVALID_HANDLE_VALUE)
	m_Thread=NULL;
if(!m_Thread)
	throw InvalidArgumentException();
s_Tasks.add(m_Id, this);
}


//================
// Common Private
//================

VOID Task::RunDeferred(Handle<Task> task)
{
DispatchedQueue::Append(task, [task]()
	{
	DispatchedQueue::Append(task, [task]()
		{
		ResumeThread(task->m_Thread);
		});
	});
}

DWORD WINAPI Task::TaskProc(VOID* param)
{
Handle<Task> task=(Task*)param;
task->m_This=nullptr;
Status status=Status::Success;
try
	{
	task->Run();
	}
catch(Exception& e)
	{
	status=e.GetStatus();
	}
WriteLock lock(task->m_Mutex);
task->m_Status=status;
task->m_Done.Trigger();
if(task->m_Then)
	{
	DispatchedQueue::Append(task->m_Then);
	task->m_Then=nullptr;
	}
lock.Unlock();
task=nullptr;
return 0;
}

shared_map<DWORD, Task*> Task::s_Tasks;

}
