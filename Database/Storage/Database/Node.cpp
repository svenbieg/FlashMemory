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
#include "Storage/Database/Entry.h"
#include "Storage/Database/Database.h"
#include "Storage/Database/NodeOperation.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"

using namespace Concurrency;
using namespace Devices::System;
using namespace Storage::Encoding;
using namespace Storage::Streams;
using namespace Storage::Xml;


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

Handle<Node> Node::Create(Database* database, UINT block)
{
WriteLock lock(database->m_Mutex);
Node* found=nullptr;
if(database->m_Nodes.try_get(block, &found))
	return found;
auto node=Object::Create<Node>(database, block);
database->m_Nodes.add(block, node);
return node;
}

Node::~Node()
{
ClearUpdate();
}


//========
// Common
//========

VOID Node::AppendChild(XmlNode* child)
{
auto node=dynamic_cast<Node*>(child);
if(!node)
	throw InvalidArgumentException();
auto editor=m_Database->Edit();
AppendChild(editor, node);
editor->Flush();
}

VOID Node::AppendChild(Editor* editor, Node* child)
{
WriteLock lock(m_Mutex);
AppendChildInternal(child);
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	Update<NodeOperationChildAppend>(&m_Update, child);
editor->Invalidate(this);
lock.Unlock();
Changed(this);
}

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
if(!ClearInternal())
	return false;
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	Update<NodeOperationClear>(&m_Update);
editor->Invalidate(this);
lock.Unlock();
Changed(this);
return true;
}

VOID Node::CopyFrom(XmlNode* copy)
{
auto editor=m_Database->Edit();
CopyFrom(editor, copy);
editor->Flush();
}

VOID Node::CopyFrom(Editor* editor, XmlNode* copy)
{
if(!copy)
	throw InvalidArgumentException();
Clear(editor);
auto tag=copy->GetTag();
SetTag(editor, tag);
for(auto it=copy->GetAttributes(); it->HasCurrent(); it->MoveNext())
	{
	auto key=it->GetKey();
	auto value=it->GetValue();
	SetAttribute(editor, key, value);
	}
auto value=copy->GetValue();
if(value)
	{
	SetValue(editor, value);
	}
else
	{
	for(auto it=copy->GetChildren(); it->HasCurrent(); it->MoveNext())
		{
		auto child=Node::Create(m_Database);
		child->CopyFrom(it->GetCurrent());
		AppendChild(editor, child);
		}
	}
}

Handle<Node> Node::GetChildAt(UINT pos)
{
ReadLock lock(m_Mutex);
auto child=m_Children.get_at(pos);
return child.As<Node>();
}

Handle<NodeChildIterator> Node::GetChildren()
{
return new NodeChildIterator(this);
}

VOID Node::InsertChildAt(UINT pos, XmlNode* child)
{
auto node=dynamic_cast<Node*>(child);
if(!node)
	throw InvalidArgumentException();
auto editor=m_Database->Edit();
InsertChildAt(editor, pos, node);
editor->Flush();
}

VOID Node::InsertChildAt(Editor* editor, UINT pos, Node* child)
{
WriteLock lock(m_Mutex);
InsertChildInternal(pos, child);
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	Update<NodeOperationChildInsert>(&m_Update, pos, child);
editor->Invalidate(this);
lock.Unlock();
Changed(this);
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
RemoveAttributeInternal(pos);
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	Update<NodeOperationAttributeRemove>(&m_Update, pos);
editor->Invalidate(this);
lock.Unlock();
Changed(this);
return true;
}

VOID Node::RemoveChildAt(UINT pos)
{
auto editor=m_Database->Edit();
RemoveChildAt(editor, pos);
editor->Flush();
}

VOID Node::RemoveChildAt(Editor* editor, UINT pos)
{
WriteLock lock(m_Mutex);
RemoveChildInternal(pos);
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	Update<NodeOperationChildRemove>(&m_Update, pos);
editor->Invalidate(this);
lock.Unlock();
Changed(this);
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
	SetAttributeInternal(pos, value);
	if(FlagHelper::Get(m_Flags, NodeFlags::Update))
		Update<NodeOperationAttributeSet>(&m_Update, pos, value);
	}
else
	{
	SetAttributeInternal(key, value);
	if(FlagHelper::Get(m_Flags, NodeFlags::Update))
		Update<NodeOperationAttributeSet>(&m_Update, key, value);
	}
editor->Invalidate(this);
lock.Unlock();
Changed(this);
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
if(!SetTagInternal(tag))
	return false;
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	Update<NodeOperationTagSet>(&m_Update, tag);
editor->Invalidate(this);
lock.Unlock();
Changed(this);
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
if(!SetValueInternal(value))
	return false;
if(FlagHelper::Get(m_Flags, NodeFlags::Update))
	Update<NodeOperationValueSet>(&m_Update, value);
editor->Invalidate(this);
lock.Unlock();
Changed(this);
return true;
}


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database, UINT block_id):
Node(database, nullptr)
{
ReadFromBlock(block_id);
}

Node::Node(Database* database, Handle<String> tag):
XmlNode(nullptr, tag),
m_BlockId(-1),
m_BlockPosition(0),
m_Database(database),
m_Flags(NodeFlags::None),
m_Update(nullptr)
{}


//==================
// Common Protected
//==================

Handle<XmlNode> Node::CreateNode()
{
return Node::Create(m_Database);
}

UINT Node::Release()noexcept
{
if(!m_BlockId)
	return XmlNode::Release();
WriteLock lock(m_Database->m_Mutex);
UINT ref_count=Cpu::InterlockedDecrement(&m_ReferenceCount);
if(ref_count==0)
	{
	m_Database->m_Nodes.remove(m_BlockId);
	delete this;
	}
return ref_count;
}


//================
// Common Private
//================

VOID Node::ClearUpdate()
{
auto op=m_Update;
while(op)
	{
	auto next=op->m_Next;
	delete op;
	op=next;
	}
m_Update=nullptr;
}

VOID Node::ReadFromBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto entry=Entry::Create(volume, block_id, NODE_ID);
StreamReader reader(entry);
NodeOperation::ReadFromStream(reader, this);
m_BlockId=block_id;
m_BlockPosition=entry->GetPosition();
Validate(this);
}

template <class _op_t, class... _args_t> VOID Node::Update(NodeOperation** next_ptr, _args_t... args)
{
while(*next_ptr)
	next_ptr=&(*next_ptr)->m_Next;
*next_ptr=new _op_t(args...);
}

VOID Node::Validate(Node* node)
{
FlagHelper::Set(m_Flags, NodeFlags::Update);
for(auto child: m_Children)
	Validate(dynamic_cast<Node*>((XmlNode*)child));
}

VOID Node::WriteToBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto entry=Entry::Create(volume, block_id, NODE_ID, FileCreateMode::CreateAlways);
StreamWriter writer(entry);
NodeOperation::WriteToStream(writer, this);
entry->Flush();
m_BlockId=block_id;
m_BlockPosition=entry->GetPosition();
}

}}