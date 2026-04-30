//========
// Node.h
//========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Collections/Array.h"
#include "Storage/Xml/XmlNode.h"
#include "Storage/Block.h"
#include "Storage/File.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//======================
// Forward-Declarations
//======================

class Database;


//======
// Node
//======

class Node: public Xml::XmlNode
{
public:
	// Using
	using SkipBitArray=Collections::Array<UINT>;

	// Friends
	friend Database;
	friend Object;

protected:
	// Con-/Destructors
	Node(Database* Database);
	Node(Database* Database, UINT Block);
	static inline Handle<Node> Create(Database* Database)
		{
		return Object::Create<Node>(Database);
		}
	static inline Handle<Node> Create(Database* Database, UINT Block)
		{
		return Object::Create<Node>(Database, Block);
		}

	// Common
	Handle<Database> m_Database;

private:
	// Common
	UINT ReadFromBlock(Block* Block);
	UINT ReadUpdates(Block* Block);
	UINT WriteToBlock(UINT Block);
	UINT SkipPages();
	UINT m_BlockId;
	UINT m_BlockPosition;
	Handle<SkipBitArray> m_SkipBits;
};

}}