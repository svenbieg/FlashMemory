//=========
// Entry.h
//=========

#pragma once


//=======
// Using
//=======

#include "Concurrency/WriteLock.h"
#include "Storage/Database/Updates/EntryUpdate.h"
#include "Storage/Volume.h"


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
public:
	// Using
	using EntryUpdate=Storage::Database::Updates::EntryUpdate;
	using InputStream=Storage::Streams::InputStream;
	using Mutex=Concurrency::Mutex;
	using OutputStream=Storage::Streams::OutputStream;
	using WriteLock=Concurrency::WriteLock;

	// Friends
	friend Database;
	friend Editor;
	friend EntryUpdate;

	// Con-/Destructors
	~Entry();

protected:
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
	VOID ClearUpdate();
	Handle<Volume> GetVolume()const;
	virtual VOID Invalidate(Editor* Editor);
	UINT Release()noexcept override;
	virtual SIZE_T WriteEntry(OutputStream* Stream)=0;
	SIZE_T WriteToBlock(UINT Block);
	SIZE_T WriteUpdates(OutputStream* Stream);
	UINT m_Block;
	Handle<Database> m_Database;
	UINT m_Id;
	Mutex m_Mutex;
	UINT m_Size;
	EntryUpdate* m_Update;

private:
	// Common
	static Mutex& GetEntriesMutex(Database* Database);
	static Entry* GetEntry(Database* Database, UINT Block);
	static VOID SetEntry(Database* Database, UINT Block, Entry* Entry);
};

}}