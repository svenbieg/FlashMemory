//===============
// Application.h
//===============

#pragma once


//=======
// Using
//=======

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
	using Volume=Storage::Volume;

	// Con-/Destructors
	static inline Handle<Application> Create() { return Object::Create<Application>(); }

private:
	// Con-/Destructors
	Application();

	// Common
	VOID PrintBuffer(BYTE const* Buffer, UINT Size);
	VOID PrintPage(Page* Page);
	Handle<Database> m_Database;
	Handle<Volume> m_Volume;
};

}