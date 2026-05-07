//=========
// Entry.h
//=========

#pragma once


//=======
// Using
//=======

#include "Concurrency/WriteLock.h"
#include "Storage/Database/Updates/SkipBits.h"
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


//=============
// Create-Mode
//=============

enum class EntryCreateMode
{
CreateNew
};


//=======
// Entry
//=======

class Entry: public Object
{
protected:
	// Using
	using Mutex=Concurrency::Mutex;
	using SkipBits=Storage::Database::Updates::SkipBits;
	using WriteLock=Concurrency::WriteLock;

	// Con-/Destructors
	Entry(Database* Database);
	Entry(Database* Database, UINT Block);
	Entry(Database* Database, UINT Block, EntryCreateMode Create);
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
	template <class _entry_t> static Handle<_entry_t> Create(Database* Database, UINT Block, EntryCreateMode Create)
		{
		auto& mutex=GetEntriesMutex(Database);
		WriteLock lock(mutex);
		auto open=GetEntry(Database, Block);
		if(open)
			throw AlreadyExistsException();
		auto entry=Object::Create<_entry_t>(Database, Block, Create);
		SetEntry(Database, Block, entry);
		return entry;
		}

	// Common
	virtual VOID Invalidate(Editor* Editor);
	UINT Release()noexcept override;
	Handle<Block> m_Block;
	UINT m_BlockId;
	UINT m_BlockPosition;
	Handle<Database> m_Database;
	Mutex m_Mutex;
	Handle<SkipBits> m_SkipBits;

private:
	// Common
	static Mutex& GetEntriesMutex(Database* Database);
	static Entry* GetEntry(Database* Database, UINT Block);
	static VOID SetEntry(Database* Database, UINT Block, Entry* Entry);
};

}}