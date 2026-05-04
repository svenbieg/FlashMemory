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
class Editor;
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
	static Handle<Node> Create(Database* Database, UINT Block);
	~Node();

	// Common
	VOID AppendChild(XmlNode* Child)override;
	VOID AppendChild(Editor* Editor, Node* Child);
	BOOL Clear()override;
	BOOL Clear(Editor* Editor);
	VOID CopyFrom(XmlNode* Node)override;
	VOID CopyFrom(Editor* Editor, XmlNode* Node);
	Handle<Node> GetChildAt(UINT Position);
	Handle<NodeChildIterator> GetChildren();
	VOID InsertChildAt(UINT Position, XmlNode* Child)override;
	VOID InsertChildAt(Editor* Editor, UINT Position, Node* Child);
	BOOL RemoveAttribute(Handle<String> Key)override;
	BOOL RemoveAttribute(Editor* Editor, Handle<String> Key);
	VOID RemoveChildAt(UINT Position)override;
	VOID RemoveChildAt(Editor* Editor, UINT Position);
	BOOL SetAttribute(Handle<String> Key, Handle<String> Value)override;
	inline BOOL SetAttribute(Handle<String> Key, INT64 Value)
		{
		return SetAttribute(Key, String::Create("%i", Value));
		}
	BOOL SetAttribute(Editor* Editor, Handle<String> Key, Handle<String> Value);
	inline BOOL SetAttribute(Editor* Editor, Handle<String> Key, INT64 Value)
		{
		return SetAttribute(Editor, Key, String::Create("%i", Value));
		}
	BOOL SetTag(Handle<String> Tag)override;
	BOOL SetTag(Editor* Editor, Handle<String> Tag);
	BOOL SetValue(Handle<String> Value)override;
	BOOL SetValue(Editor* Editor, Handle<String> Value);

protected:
	// Con-/Destructors
	Node(Database* Database, UINT Block);
	Node(Database* Database, Handle<String> Tag=nullptr);
	static inline Handle<Node> Create(Database* Database, Handle<String> Tag=nullptr)
		{
		return Object::Create<Node>(Database, Tag);
		}

	// Common
	Handle<XmlNode> CreateNode()override;
	UINT Release()noexcept override;

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