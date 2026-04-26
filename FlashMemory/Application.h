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

	// Con-/Destructors
	static inline Handle<Application> Create() { return Object::Create<Application>(); }

private:
	// Con-/Destructors
	Application();

	// Common
	Handle<Database> m_Database;
};

}