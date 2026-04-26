//===========
// Console.h
//===========

#pragma once


//=======
// Using
//=======

#include "Collections/map.hpp"
#include "Concurrency/Task.h"
#include "Devices/Serial/SerialPort.h"
#include "Function.h"
#include "Global.h"
#include "StringBuilder.h"


//===========
// Namespace
//===========

namespace UI {


//=========
// Console
//=========

class Console: public Global<Console>
{
public:
	// Using
	using SerialPort=Devices::Serial::SerialPort;

	// Common
	VOID AddCommand(Handle<String> Command, Function<VOID()> Function);
	static inline Handle<Console> Get() { return Global::Create(); }
	static VOID Pause();
	static VOID Print(LPCSTR Text);
	static VOID Print(LPCWSTR Text);
	template <class... _args_t> static VOID Print(LPCSTR Format, _args_t... Arguments)
		{
		VariableArgument args[]={ Arguments... };
		VariableArguments vargs(args, TypeHelper::ArraySize(args));
		auto str=String::Create(Format, vargs);
		Print(str->Begin());
		}

private:
	// Con-/Destructors
	friend Object;
	Console();
	static Global<Console> s_Current;

	// Common
	Collections::map<Handle<String>, Function<VOID()>> m_Commands;
	Concurrency::Mutex m_Mutex;
	Handle<SerialPort> m_SerialPort;
	StringBuilder m_StringBuilder;
	Handle<Console> m_This;
};

}