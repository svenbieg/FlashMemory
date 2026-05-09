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
	auto child=GetChildAt(pos);
	child->Clear();
	editor->Free(child->m_BlockId);
	}
m_Value=nullptr;
if(m_BlockId!=-1)
	NodeUpdateClear::Create(this);
Invalidate(editor);
return true;
}

Handle<String> Node::GetAttribute(Handle<String> key)
{
ReadLock lock(m_Mutex);
return m_Attributes.get(key);
}

Handle<Node> Node::GetChildAt(UINT pos)
{
ReadLock lock(m_Mutex);
UINT block_id=m_Children.get_at(pos);
return Node::Create(m_Database, block_id);
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
if(!m_Attributes.remove(key))
	return false;
if(m_BlockId!=-1)
	NodeUpdateAttributeRemove::Create(this, key);
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
UINT block_id=child->m_BlockId;
if(!m_Children.remove(block_id))
	throw NotFoundException();
if(m_BlockId!=-1)
	NodeUpdateChildRemove::Create(this, block_id);
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
if(!m_Attributes.set(key, value))
	return false;
if(m_BlockId!=-1)
	NodeUpdateAttributeSet::Create(this, key, value);
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
if(m_BlockId!=-1)
	NodeUpdateTagSet::Create(this, tag);
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
if(m_BlockId!=-1)
	NodeUpdateValueSet::Create(this, value);
Invalidate(editor);
return true;
}


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database, UINT block_id):
Entry(database, block_id)
{
if(m_BlockId==-1)
	return;
auto block=Block::Create(m_Database, m_BlockId);
ReadEntry(block);
m_SkipBits.Skip(block);
ReadUpdate(block);
m_BlockPosition=block->GetPosition();
}


//==================
// Common Protected
//==================

SIZE_T Node::ReadEntry(Block* block)
{
SIZE_T size=0;
size+=block->Read(&m_Id, sizeof(UINT));
if(m_Id!=NODE_ID)
	throw InvalidArgumentException();
size+=NodeUpdate::ReadFromBlock(block, this);
return size;
}

SIZE_T Node::ReadUpdate(Block* block)
{
return NodeUpdate::ReadFromBlock(block, this, &m_Update);
}

SIZE_T Node::WriteEntry(Block* block)
{
SIZE_T size=0;
m_Id=NODE_ID;
size+=OutputStream::Write(block, &m_Id, sizeof(UINT));
size+=NodeUpdate::WriteToStream(block, this);
return size;
}

}}