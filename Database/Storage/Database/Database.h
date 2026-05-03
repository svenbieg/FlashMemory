//============
// Database.h
//============

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Concurrency/Mutex.h"
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
	// Friends
	friend Editor;
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
	VOID Initialize();
	Handle<Node> ReadHeader();
	VOID ValidateHeader(Node* Header);
	Handle<Node> m_Header;
	Concurrency::Mutex m_Mutex;
	Handle<Volume> m_Volume;
};

}}