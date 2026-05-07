//=========
// Entry.h
//=========

#pragma once


//=======
// Using
//=======

#include "Concurrency/WriteLock.h"
#include "Storage/Database/Updates/SkipBits.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//====
// ID
//====

constexpr UINT ENTRY_ID(UINT Value)
{
return TypeHelper::BigEndian(Value);
}


//======================
// Forward-Declarations
//======================

class Database;
class Editor;


//=======
// Entry
//=======

class Entry: public Object
{
protected:
	// Using
	using Mutex=Concurrency::Mutex;
	using WriteLock=Concurrency::WriteLock;
	using SkipBits=Storage::Database::Updates::SkipBits;

	// Con-/Destructors
	Entry(Database* Database, UINT Block=-1);
	template <class _entry_t> static Handle<_entry_t> Create(Database* Database, UINT Block)
		{
		if(Block==-1)
			return Object::Create<_entry_t>(Database, Block);
		auto& mutex=GetEntriesMutex(Database);
		WriteLock lock(mutex);
		auto open=GetEntry(Database, Block);
		if(open)
			{
			auto entry=dynamic_cast<_entry_t*>(open);
			if(!entry)
				throw InvalidArgumentException();
			return entry;
			}
		auto entry=Object::Create<_entry_t>(Database, Block);
		SetEntry(Database, Block, entry);
		return entry;
		}

	// Common
	virtual VOID Invalidate(Editor* Editor);
	UINT Release()noexcept override;
	UINT m_BlockId;
	UINT m_BlockPosition;
	Handle<Database> m_Database;
	Mutex m_Mutex;

private:
	// Common
	static Mutex& GetEntriesMutex(Database* Database);
	static Entry* GetEntry(Database* Database, UINT Block);
	static VOID SetEntry(Database* Database, UINT Block, Entry* Entry);
};

}}