//==========
// Node.cpp
//==========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Node.h"


//=======
// Using
//=======

#include "Concurrency/ReadLock.h"
#include "Concurrency/WriteLock.h"
#include "Storage/Database/Block.h"
#include "Storage/Database/Database.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"

using namespace Concurrency;
using namespace Devices::System;
using namespace Storage::Database::Updates;
using namespace Storage::Encoding;
using namespace Storage::Streams;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//==========
// Settings
//==========

static const UINT NODE_ID='NODE';


//==================
// Con-/Destructors
//==================

Node::~Node()
{
ClearUpdate();
}


//========
// Common
//========

BOOL Node::Clear()
{
auto editor=m_Database->Edit();
BOOL cleared=Clear(editor);
if(cleared)
	editor->Flush();
return cleared;
}

BOOL Node::Clear(Editor* editor)
{
WriteLock lock(m_Mutex);
return ClearInternal(editor);
}

Handle<String> Node::GetAttribute(Handle<String> key)
{
ReadLock lock(m_Mutex);
return m_Attributes.get(key);
}

Handle<Node> Node::GetChildAt(UINT pos)
{
ReadLock lock(m_Mutex);
return GetChildInternal(pos);
}

BOOL Node::RemoveAttribute(Handle<String> key)
{
auto editor=m_Database->Edit();
BOOL removed=RemoveAttribute(editor, key);
if(removed)
	editor->Flush();
return removed;
}

BOOL Node::RemoveAttribute(Editor* editor, Handle<String> key)
{
WriteLock lock(m_Mutex);
UINT pos=0;
if(!m_Attributes.index_of(key, &pos))
	return false;
m_Attributes.remove_at(pos);
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateAttributeRemove>(&m_Update, pos);
editor->Invalidate(this);
return true;
}

VOID Node::RemoveChild(Handle<Node> child)
{
auto editor=m_Database->Edit();
RemoveChild(editor, child);
editor->Flush();
}

VOID Node::RemoveChild(Editor* editor, Handle<Node> child)
{
WriteLock lock(m_Mutex);
UINT pos=0;
if(!m_Children.index_of(child, &pos))
	throw NotFoundException();
FreeChild(editor, child);
m_Children.remove_at(pos);
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateChildRemove>(&m_Update, pos);
editor->Invalidate(this);
}

BOOL Node::SetAttribute(Handle<String> key, Handle<String> value)
{
auto editor=m_Database->Edit();
BOOL set=SetAttribute(editor, key, value);
if(set)
	editor->Flush();
return set;
}

BOOL Node::SetAttribute(Editor* editor, Handle<String> key, Handle<String> value)
{
WriteLock lock(m_Mutex);
UINT pos=0;
if(m_Attributes.index_of(key, &pos))
	{
	m_Attributes.set(key, value);
	if(FlagHelper::Get(m_Flags, NodeFlags::Update))
		NodeUpdate::Create<NodeUpdateAttributeSet>(&m_Update, pos, value);
	}
else
	{
	m_Attributes.set(key, value);
	if(FlagHelper::Get(m_Flags, NodeFlags::Update))
		NodeUpdate::Create<NodeUpdateAttributeSet>(&m_Update, key, value);
	}
editor->Invalidate(this);
return true;
}

BOOL Node::SetTag(Handle<String> tag)
{
auto editor=m_Database->Edit();
BOOL set=SetTag(editor, tag);
if(set)
	editor->Flush();
return set;
}

BOOL Node::SetTag(Editor* editor, Handle<String> tag)
{
WriteLock lock(m_Mutex);
if(m_Tag==tag)
	return false;
m_Tag=tag;
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateTagSet>(&m_Update, tag);
editor->Invalidate(this);
return true;
}

BOOL Node::SetValue(Handle<String> value)
{
auto editor=m_Database->Edit();
BOOL set=SetValue(editor, value);
if(set)
	editor->Flush();
return set;
}

BOOL Node::SetValue(Editor* editor, Handle<String> value)
{
WriteLock lock(m_Mutex);
if(m_Value==value)
	return false;
m_Value=value;
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateValueSet>(&m_Update, value);
editor->Invalidate(this);
return true;
}


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database, UINT block):
Entry(database, block),
m_Flags(NodeFlags::None),
m_Parent(nullptr),
m_Update(nullptr)
{}

Node::Node(Node* parent, Handle<String> tag):
Entry(parent->m_Database, -1),
m_Flags(NodeFlags::None),
m_Parent(parent),
m_Update(nullptr)
{
m_Parent->m_Children.append(this);
if(FlagHelper::Get(m_Parent->m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateChildAppend>(&m_Parent->m_Update, this);
}

Handle<Node> Node::Create(Database* database, UINT block)
{
return database->CreateEntry<Node>(block);
}


//================
// Common Private
//================

BOOL Node::ClearInternal(Editor* editor)
{
BOOL clear=false;
clear|=m_Attributes;
clear|=m_Children;
clear|=m_Value;
if(!clear)
	return false;
m_Attributes.clear();
UINT child_count=m_Children.get_count();
for(UINT pos=0; pos<child_count; pos++)
	{
	auto child=GetChildInternal(pos);
	FreeChild(editor, child);
	}
m_Value=nullptr;
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateClear>(&m_Update);
editor->Invalidate(this);
return true;
}

VOID Node::ClearUpdate()
{
auto update=m_Update;
while(update)
	{
	auto next=update->m_Next;
	delete update;
	update=next;
	}
m_Update=nullptr;
}

VOID Node::FreeChild(Editor* editor, Node* child)
{
WriteLock child_lock(child->m_Mutex);
child->ClearInternal(editor);
child->ClearUpdate();
child->m_Parent=nullptr;
if(child->m_Block)
	{
	editor->Free(child->m_Block);
	child->m_Block=-1;
	}
}

Handle<Node> Node::GetChildInternal(UINT pos)
{
auto child=m_Children.get_at(pos);
WriteLock lock(child->m_Mutex);
Handle<String> at;
if(!child->m_Attributes.try_get("@", &at))
	return child;
child->m_Attributes.remove("@");
INT rel=0;
at->Scan("%i", &rel);
UINT block=m_Block+rel;
child->ReadFromBlock(block);
return child;
}

VOID Node::ReadFromBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto block=Block::Create(volume, block_id);
StreamReader reader(block);
UINT id=0;
reader.Read(&id, sizeof(UINT));
if(id!=NODE_ID)
	throw InvalidArgumentException();
auto skip_bits=block->ReadSkipBits();
block->SkipPages(skip_bits);
NodeUpdate::ReadFromStream(reader, this);
m_Block=block_id;
m_BlockPosition=block->GetPosition();
}

VOID Node::WriteToBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto block=Block::Create(volume, block_id);
StreamWriter writer(block);
NodeUpdate::WriteToStream(writer, this);
block->Flush();
m_Block=block_id;
m_BlockPosition=block->GetPosition();
}

}}