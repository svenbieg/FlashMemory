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
	editor->Free(child->m_Block);
	}
m_Value=nullptr;
if(m_Block!=-1)
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
if(m_Block!=-1)
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
UINT block_id=child->m_Block;
if(!m_Children.remove(block_id))
	throw NotFoundException();
if(m_Block!=-1)
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
if(m_Block!=-1)
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
if(m_Block!=-1)
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
if(m_Block!=-1)
	NodeUpdateValueSet::Create(this, value);
Invalidate(editor);
return true;
}


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database):
Entry(database, -1, NODE_ID)
{}

Node::Node(Database* database, UINT block_id):
Entry(database, block_id, NODE_ID)
{
auto volume=GetVolume();
auto block=Block::Create(volume, m_Block);
ReadFromStream(block);
block->Skip();
NodeUpdate::ReadFromStream(block, this, &m_Update);
m_Size=block->GetPosition();
}


//==================
// Common Protected
//==================

SIZE_T Node::ReadFromStream(InputStream* stream)
{
SIZE_T size=0;
size+=Entry::ReadFromStream(stream);
if(m_Id!=NODE_ID)
	throw NotFoundException();
size+=NodeUpdate::ReadFromStream(stream, this);
for(auto const& attr: m_Attributes)
	m_AttributeIndex.add(attr.get_key());
return size;
}

SIZE_T Node::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
size+=Entry::WriteToStream(stream);
size+=NodeUpdateTagSet::WriteToStream(stream, m_Tag);
for(auto const& attr: m_Attributes)
	size+=NodeUpdateAttributeSet::WriteToStream(stream, attr.get_key(), attr.get_value());
if(m_Value)
	{
	size+=NodeUpdateValueSet::WriteToStream(stream, m_Value);
	}
else
	{
	for(auto const& child: m_Children)
		size+=NodeUpdateChildAppend::WriteToStream(stream, child);
	}
auto update=NodeUpdateId::None;
size+=stream->Write(&update, sizeof(NodeUpdateId));
return size;
}

}}