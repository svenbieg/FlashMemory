//=========
// Entry.h
//=========

#pragma once


//=======
// Using
//=======

#include "Concurrency/Mutex.h"
#include "Storage/Database/Block.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//======================
// Forward-Declarations
//======================

class Database;


//=======
// Entry
//=======

class Entry: public Object
{
protected:
	// Using
	using Mutex=Concurrency::Mutex;

	// Con-/Destructors
	Entry(Database* Database, UINT Block);

	// Common
	UINT Release()noexcept override;
	UINT m_Block;
	UINT m_BlockPosition;
	Handle<Database> m_Database;
	Mutex m_Mutex;
};

}}