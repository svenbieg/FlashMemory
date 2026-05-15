//========
// Node.h
//========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Collections/list.hpp"
#include "Collections/map.hpp"
#include "Storage/Database/Updates/NodeUpdate.h"
#include "Storage/Database/Entry.h"
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

class Node: public Entry
{
public:
	// Using
	using AttributeIndex=Collections::index<Handle<String>, UINT>;
	using AttributeMap=Collections::map<Handle<String>, Handle<String>, UINT>;
	using ChildList=Collections::list<Handle<Node>, UINT>;
	using NodeUpdate=Storage::Database::Updates::NodeUpdate;

	// Friends
	friend Database;
	friend NodeUpdate;
	friend Object;

	// Common
	BOOL Clear();
	BOOL Clear(Editor* Editor);
	Handle<String> GetAttribute(Handle<String> Key);
	Handle<Node> GetChild(Handle<String> Tag);
	Handle<Node> GetChildAt(UINT Position);
	Handle<String> GetTag();
	SIZE_T ReadFromStream(InputStream* Stream);
	BOOL RemoveAttribute(Handle<String> Key);
	BOOL RemoveAttribute(Editor* Editor, Handle<String> Key);
	VOID RemoveChild(Handle<Node> Child);
	VOID RemoveChild(Editor* Editor, Handle<Node> Child);
	BOOL SetAttribute(Handle<String> Key, Handle<String> Value);
	BOOL SetAttribute(Editor* Editor, Handle<String> Key, Handle<String> Value);
	BOOL SetTag(Handle<String> Tag);
	BOOL SetTag(Editor* Editor, Handle<String> Tag);
	BOOL SetValue(Handle<String> Value);
	BOOL SetValue(Editor* Editor, Handle<String> Value);
	SIZE_T WriteToStream(OutputStream* Stream);

protected:
	// Con-/Destructors
	Node(Database* Database, UINT Id);
	Node(Database* Database, Handle<String> Tag=nullptr);
	Node(Node* Parent, Handle<String> Tag=nullptr);
	static inline Handle<Node> Create(Database* Database, UINT Block)
		{
		return Entry::Create<Node>(Database, Block);
		}
	static inline Handle<Node> Create(Database* Database, Handle<String> Tag=nullptr)
		{
		return Object::Create<Node>(Database, Tag);
		}
	static inline Handle<Node> Create(Node* Parent, Handle<String> Tag=nullptr)
		{
		assert(Parent);
		return Object::Create<Node>(Parent, Tag);
		}

	// Common
	SIZE_T ReadEntry(InputStream* Stream)override;
	SIZE_T WriteEntry(OutputStream* Stream)override;

private:
	// Settings
	static const UINT NODE_TYPE=ENTRY_TYPE('NODE');

	// Common
	BOOL ClearInternal(Editor* Editor);
	AttributeIndex m_AttributeIndex;
	AttributeMap m_Attributes;
	ChildList m_Children;
	Handle<String> m_Tag;
	Handle<String> m_Value;
};

}}