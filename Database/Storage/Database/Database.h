//============
// Database.h
//============

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Collections/map.hpp"
#include "Concurrency/WriteLock.h"
#include "Storage/Database/Editor.h"
#include "Storage/File.h"
#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//==========
// Database
//==========

class Database: public Object
{
public:
	// Using
	using EntryMap=Collections::map<UINT, Entry*>;
	using WriteLock=Concurrency::WriteLock;

	// Friends
	friend Editor;
	friend Entry;
	friend Node;
	friend Object;

	// Con-/Destructors
	static inline Handle<Database> Create(Volume* Volume, FileCreateMode Create=FileCreateMode::OpenExisting) { return Object::Create<Database>(Volume, Create); }

	// Common
	inline Handle<Editor> Edit() { return Editor::Create(this); }
	inline Handle<Volume> GetVolume()const { return m_Volume; }

private:
	// Con-/Destructors
	Database(Volume* Volume, FileCreateMode Create);

	// Common
	template <class _entry_t> Handle<_entry_t> CreateEntry(UINT Block)
		{
		WriteLock lock(m_Mutex);
		Entry* entry=nullptr;
		if(m_Entries.try_get(Block, &entry))
			{
			entry=dynamic_cast<_entry_t*>(entry);
			if(!entry)
				throw InvalidArgumentException();
			return entry;
			}
		auto created=Object::Create<_entry_t>(this, Block);
		m_Entries.add(Block, created);
		return created;
		}
	VOID Initialize();
	Handle<Node> ReadHeader();
	VOID ValidateHeader(Node* Header);
	EntryMap m_Entries;
	Handle<Node> m_Header;
	Concurrency::Mutex m_Mutex;
	Handle<Volume> m_Volume;
};

}}