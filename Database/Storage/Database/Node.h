//========
// Node.h
//========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/StreamBuffer.h"
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
class NodeOperation;


//======
// Node
//======

class Node: public Xml::XmlNode
{
public:
	// Using
	using StreamBuffer=Storage::Streams::StreamBuffer;

	// Friends
	friend Database;
	friend NodeOperation;
	friend Object;
	
	// Con-/Destructors
	static inline Handle<Node> Create(Handle<String> Tag=nullptr)
		{
		return Object::Create<Node>(nullptr, Tag);
		}

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
	Node* GetBlockNode();
	VOID ReadFromBlock(UINT Block);
	UINT ReadUpdates(InputStream* Stream);
	VOID WriteToBlock(UINT Block);
	UINT m_BlockId;
	UINT m_BlockPosition;
	Database* m_Database;
	NodeOperation* m_Operation;
};

}}