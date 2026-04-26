//========
// Node.h
//========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Handle.h"


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


//======
// Node
//======

class Node: public Object
{
public:
	// Friends
	friend Editor;

protected:
	// Con-/Destructors
	Node(Database* Database, UINT Block);

	// Common
	Handle<Database> m_Database;
	UINT m_Block;
	UINT m_Read;
	UINT m_Written;
};

}}