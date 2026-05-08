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
#include "Storage/Database/Map.h"
#include "Storage/Database/Node.h"
#include "Storage/Encoding/Unsigned.h"
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
	using Unsigned=Storage::Encoding::Unsigned;
	using BlockMap=Map<Unsigned, Unsigned>;
	using EntryMap=Collections::map<UINT, Entry*>;
	using Mutex=Concurrency::Mutex;
	using WriteLock=Concurrency::WriteLock;

	// Friends
	friend Editor;
	friend Entry;
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
	VOID Initialize();
	Handle<Node> ReadHeader();
	VOID ValidateHeader(Node* Header);
	WORD m_Alignment;
	EntryMap m_Entries;
	Mutex m_EntriesMutex;
	Handle<Node> m_Header;
	Handle<BlockMap> m_MapFree;
	Mutex m_Mutex;
	Handle<Volume> m_Volume;
};

}}