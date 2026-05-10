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
	using ChildList=Collections::list<UINT, UINT>;
	using NodeUpdate=Storage::Database::Updates::NodeUpdate;

	// Friends
	friend Database;
	friend NodeUpdate;
	friend Object;

	// Common
	BOOL Clear();
	BOOL Clear(Editor* Editor);
	Handle<String> GetAttribute(Handle<String> Key);
	Handle<Node> GetChildAt(UINT Position);
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

protected:
	// Con-/Destructors
	Node(Database* Database, UINT Block);
	static inline Handle<Node> Create(Database* Database, UINT Block=-1)
		{
		return Entry::Create<Node>(Database, Block);
		}

	// Common
	SIZE_T WriteEntry(Block* Block)override;

private:
	// Settings
	static const UINT NODE_ID=ENTRY_ID('NODE');

	// Common
	BOOL ClearInternal(Editor* Editor);
	AttributeIndex m_AttributeIndex;
	AttributeMap m_Attributes;
	ChildList m_Children;
	Handle<String> m_Tag;
	Handle<String> m_Value;
};

}}