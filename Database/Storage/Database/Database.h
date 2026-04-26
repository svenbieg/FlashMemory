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
#include "Storage/Database/Node.h"
#include "Storage/File.h"
#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//======================
// Forward-Declarations
//======================

class Editor;


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

private:
	// Con-/Destructors
	Database(Volume* Volume, FileCreateMode Create);

	// Common
	Concurrency::Mutex m_Mutex;
	Handle<Node> m_Root;
	Handle<Volume> m_Volume;
};

}}