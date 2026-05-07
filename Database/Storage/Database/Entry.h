//=========
// Entry.h
//=========

#pragma once


//=======
// Using
//=======

#include "Concurrency/WriteLock.h"
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

	// Con-/Destructors
	Entry(Database* Database, UINT Block);
	Entry(Entry* Parent);
	template <class _entry_t> static Handle<_entry_t> Create(Database* Database, UINT Block)
		{
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
	VOID Invalidate(Editor* Editor);
	UINT Release()noexcept override;
	UINT m_Block;
	UINT m_BlockPosition;
	Handle<Database> m_Database;
	Mutex m_Mutex;
	Entry* m_Parent;

private:
	// Common
	static Mutex& GetEntriesMutex(Database* Database);
	static Entry* GetEntry(Database* Database, UINT Block);
	static VOID SetEntry(Database* Database, UINT Block, Entry* Entry);
};

}}