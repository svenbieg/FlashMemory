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
	Node(Handle<String> Tag=nullptr);
	Node(Database* Database, UINT Block);
	static inline Handle<Node> Create(Handle<String> Tag=nullptr)
		{
		return Object::Create<Node>(Tag);
		}
	static inline Handle<Node> Create(Database* Database, UINT Block)
		{
		return Object::Create<Node>(Database, Block);
		}

private:
	// Common
	Handle<SkipBitArray> CreateSkipBits(Volume* Volume);
	VOID ReadFromBlock(Database* Database, UINT Block);
	UINT ReadFromPage(Block* Block);
	UINT ReadUpdates(Block* Block);
	UINT SkipPages(Block* Block, SkipBitArray* SkipBits);
	VOID WriteToBlock(Database* Database, UINT Block);
	UINT WriteToPage(Block* Block);
	UINT m_BlockId;
	UINT m_BlockPosition;
};

}}