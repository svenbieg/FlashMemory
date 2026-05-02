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
class NodeChildIterator;
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
	friend NodeChildIterator;
	friend NodeOperation;
	friend Object;

	// Con-/Destructors
	~Node();

	// Common
	VOID AppendChild(Node* Child);
	VOID AppendChild(XmlNode* Child)override;
	BOOL Clear()override;
	VOID CopyFrom(XmlNode* Node)override;
	Handle<Node> GetChildAt(UINT Position);
	Handle<NodeChildIterator> GetChildren();
	VOID InsertChildAt(UINT Position, Node* Child);
	VOID InsertChildAt(UINT Position, XmlNode* Child)override;
	BOOL RemoveAttribute(Handle<String> Key)override;
	VOID RemoveChildAt(UINT Position)override;
	inline BOOL SetAttribute(Handle<String> Key, INT Value)
		{
		return SetAttribute(Key, String::Create("%i", Value));
		}
	inline BOOL SetAttribute(Handle<String> Key, INT64 Value)
		{
		return SetAttribute(Key, String::Create("%i", Value));
		}
	BOOL SetAttribute(Handle<String> Key, Handle<String> Value)override;
	BOOL SetTag(Handle<String> Tag)override;
	BOOL SetValue(Handle<String> Value)override;

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

protected:
	// Common
	Handle<XmlNode> CreateNode()override;

private:
	// Flags
	enum class NodeFlags: UINT
		{
		None=0,
		Update=(1<<0)
		};

	// Common
	VOID ClearUpdate();
	VOID ReadFromBlock(UINT Block);
	template <class _op_t, class... _args_t> VOID Update(NodeOperation** Next, _args_t... Arguments);
	VOID Validate(Node* Node);
	VOID WriteToBlock(UINT Block);
	UINT m_BlockId;
	UINT m_BlockPosition;
	Database* m_Database;
	NodeFlags m_Flags;
	NodeOperation* m_Update;
};


//================
// Child-Iterator
//================

class NodeChildIterator: public Storage::Xml::XmlNodeChildIterator
{
public:
	// Friends
	friend Node;

	// Access
	inline Handle<Node> GetCurrent()const
		{
		auto current=m_It.get_current();
		return current.As<Node>();
		}

private:
	// Con-/Destructors
	NodeChildIterator(Node* Node): XmlNodeChildIterator(Node) {}
};

}}