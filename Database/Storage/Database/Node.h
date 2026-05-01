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
class Editor;


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
	Node(Database* Database, UINT Block);
	Node(Database* Database, Handle<String> Tag=nullptr);
	static inline Handle<Node> Create(Database* Database, UINT Block)
		{
		return Object::Create<Node>(Database, Block);
		}
	static inline Handle<Node> Create(Database* Database, Handle<String> Tag=nullptr)
		{
		return Object::Create<Node>(Database, Tag);
		}

private:
	// Common
	Handle<SkipBitArray> CreateSkipBits(Volume* Volume, UINT Skip=0);
	Node* GetBlockNode();
	VOID OnChanged();
	VOID ReadFromBlock(UINT Block);
	UINT ReadFromPage(InputStream* Stream);
	UINT ReadUpdates(InputStream* Stream);
	UINT SkipPages(Block* Block, SkipBitArray* SkipBits);
	VOID WriteToBlock(UINT Block);
	UINT WriteToPage(OutputStream* Stream);
	UINT m_BlockId;
	UINT m_BlockPosition;
	Editor* m_Editor;
	Database* m_Database;
	UINT m_Size;
};

}}