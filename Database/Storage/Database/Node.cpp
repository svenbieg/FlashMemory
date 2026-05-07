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
#include "FlagHelper.h"

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
Invalidate(editor);
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
if(!m_Children.remove(child))
	throw NotFoundException();
FreeChild(editor, child);
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateChildRemove>(&m_Update, child);
Invalidate(editor);
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
Invalidate(editor);
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
Invalidate(editor);
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
Invalidate(editor);
return true;
}


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database):
Entry(database),
m_Flags(NodeFlags::None),
m_Update(nullptr)
{}

Node::Node(Database* database, UINT block):
Entry(database, block),
m_Flags(NodeFlags::None),
m_Update(nullptr)
{
StreamReader reader(m_Block);
UINT id=0;
reader.Read(&id, sizeof(UINT));
if(id!=NODE_ID)
	throw InvalidArgumentException();
NodeUpdate::ReadFromStream(reader, this);
m_BlockPosition=m_Block->GetPosition();
m_Block=nullptr;
}

Node::Node(Database* database, UINT block, EntryCreateMode create):
Entry(database, block, create),
m_Flags(NodeFlags::None),
m_Update(nullptr)
{}

Node::Node(Node* parent, Handle<String> tag):
Entry(parent->m_Database),
m_Flags(NodeFlags::None),
m_Update(nullptr)
{
parent->m_Children.append(this);
if(FlagHelper::Get(parent->m_Flags, NodeFlags::Update))
	NodeUpdate::Create<NodeUpdateChildAppend>(&parent->m_Update, this);
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
Invalidate(editor);
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
if(child->m_BlockId!=-1)
	{
	editor->Free(child->m_Block);
	child->m_BlockId=-1;
	}
}

Handle<Node> Node::GetChildInternal(UINT pos)
{
auto child=m_Children.get_at(pos);
auto at=child->GetAttribute("@");
if(!at)
	return child;
INT relative=0;
if(at->Scan("%i", &relative)!=1)
	throw InvalidArgumentException();
UINT block_id=m_BlockId+relative;
child=Node::Create(m_Database, block_id);
m_Children.set_at(pos, child);
return child;
}

VOID Node::WriteToBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto block=Block::Create(volume, block_id);
if(m_SkipBits)
	{
	m_SkipBits->Clear();
	}
else
	{
	m_SkipBits=SkipBits::Create(volume);
	}
m_SkipBits->WriteToStream(block);
StreamWriter writer(block);
NodeUpdate::WriteToStream(writer, this);
block->Flush();
m_BlockId=block_id;
m_BlockPosition=block->GetPosition();
}

}}