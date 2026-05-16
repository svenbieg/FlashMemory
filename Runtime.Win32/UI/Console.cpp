//=============
// Console.cpp
//=============

#include "Console.h"


//=======
// Using
//=======

using namespace Concurrency;


//==========
// Callback
//==========

BOOL WINAPI ConsoleCallback(DWORD id)
{
if(id==CTRL_CLOSE_EVENT)
	{
	DispatchedQueue::Exit();
	return false;
	}
return false;
}


//===========
// Namespace
//===========

namespace UI {


//========
// Common
//========

VOID Console::AddCommand(Handle<String> cmd, Function<VOID()> func)
{
WriteLock lock(m_Mutex);
m_Commands.add(cmd, func);
}

VOID Console::Pause()
{
HANDLE input=GetStdHandle(STD_INPUT_HANDLE);
BYTE buf[1];
DWORD read=0;
ReadConsoleA(input, buf, 1, &read, nullptr);
}

VOID Console::Print(LPCSTR text)
{
HANDLE output=GetStdHandle(STD_OUTPUT_HANDLE);
UINT len=StringHelper::Length(text);
WriteConsoleA(output, text, len, nullptr, nullptr);
}

VOID Console::Print(LPCWSTR text)
{
HANDLE output=GetStdHandle(STD_OUTPUT_HANDLE);
UINT len=StringHelper::Length(text);
WriteConsoleW(output, text, len, nullptr, nullptr);
}


//==========================
// Con-/Destructors Private
//==========================

Console::Console():
m_This(this)
{
SetConsoleCtrlHandler(ConsoleCallback, true);
}

}