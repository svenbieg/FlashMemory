//==========
// Editor.h
//==========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Storage/Database/Node.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//======================
// Forward-Declarations
//======================

class Database;


//========
// Editor
//========

class Editor: public Object
{
public:
	// Using
	using NodeIndex=Collections::index<Handle<Node>>;

	// Friends
	friend Database;
	friend Node;
	friend Object;

	// Con-/Destructors
	~Editor();

	// Common
	UINT Allocate(UINT Count=1);
	VOID Cancel();
	VOID Flush();
	VOID Free(UINT Block, UINT Count=1);

private:
	// Con-/Destructors
	Editor(Database* Database);
	static inline Handle<Editor> Create(Database* Database) { return Object::Create<Editor>(Database); }

	// Common
	NodeIndex m_ChangedNodes;
	Database* m_Database;
};

}}