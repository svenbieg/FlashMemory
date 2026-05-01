//==========
// Node.cpp
//==========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Node.h"


//=======
// Using
//=======

#include "Devices/System/Cpu.h"
#include "Storage/Database/Database.h"
#include "Storage/Database/Editor.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"

using namespace Devices::System;
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


//===========
// Operation
//===========

enum class NodeOperation: BYTE
{
None=0,
AttributeRemove,
AttributeSet,
ChildAppend,
ChildInsert,
ChildRemove,
NodeBegin,
NodeEnd,
TargetSet,
ValueSet,
Done=255
};


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database, UINT block_id):
m_BlockId(-1),
m_BlockPosition(0),
m_Database(database),
m_Editor(nullptr),
m_Size(0)
{
ReadFromBlock(block_id);
}

Node::Node(Database* database, Handle<String> tag):
XmlNode(tag),
m_BlockId(-1),
m_BlockPosition(0),
m_Database(database),
m_Editor(nullptr),
m_Size(0)
{}


//================
// Common Private
//================

Node* Node::GetBlockNode()
{
if(m_BlockId!=-1)
	return this;
auto node=this;
while(node->m_Parent)
	{
	auto parent=dynamic_cast<Node*>(node->m_Parent);
	assert(parent);
	if(parent->m_BlockId!=-1)
		return parent;
	node=parent;
	}
return nullptr;
}

VOID Node::OnChanged()
{
auto block_node=GetBlockNode();
assert(block_node);
m_Editor->m_ChangedNodes.add(block_node);
}

VOID Node::ReadFromBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto block=Block::Create(volume);
block->Seek(block_id, 0);
UINT id=0;
SIZE_T size=block->Read(&id, sizeof(UINT));
if(id!=NODE_ID)
	throw InvalidArgumentException();
auto skip_bits=block->CreateSkipBits();
size+=skip_bits->ReadFromStream(block);
size+=block->SkipPages(skip_bits);
size+=ReadFromPage(block);
size+=ReadUpdates(block);
m_BlockId=block_id;
m_BlockPosition=(UINT)size;
}

UINT Node::ReadFromPage(InputStream* stream)
{
StreamReader reader(stream);
SIZE_T size=0;
NodeOperation op;
size+=reader.Read(&op, sizeof(NodeOperation));
if(op!=NodeOperation::NodeBegin)
	throw InvalidArgumentException();
m_Tag=reader.ReadString(&size);
while(stream->Available())
	{
	NodeOperation op;
	size+=reader.Read(&op, sizeof(NodeOperation));
	if(op==NodeOperation::NodeEnd)
		break;
	switch(op)
		{
		case NodeOperation::AttributeSet:
			{
			UINT attr_id=0;
			size+=Dwarf::ReadUnsigned(stream, &attr_id);
			if(attr_id!=0)
				throw InvalidArgumentException();
			auto key=reader.ReadString(&size);
			auto value=reader.ReadString(&size);
			m_Attributes.set(key, value);
			break;
			}
		case NodeOperation::ChildAppend:
			{
			auto child=Node::Create(m_Database);
			size+=child->ReadFromPage(stream);
			child->m_Parent=this;
			m_Children.append(child);
			auto name=child->GetName();
			if(name)
				m_Index.set(name, child);
			break;
			}
		case NodeOperation::ValueSet:
			{
			m_Value=reader.ReadString(&size);
			break;
			}
		default:
			{
			throw InvalidArgumentException();
			}
		}
	}
m_Size=(UINT)size;
return size;
}

UINT Node::ReadUpdates(InputStream* stream)
{
StreamReader reader(stream);
SIZE_T size=0;
Node* node=this;
while(stream->Available())
	{
	NodeOperation op;
	size+=reader.Read(&op, sizeof(NodeOperation));
	if(op==NodeOperation::Done)
		break;
	switch(op)
		{
		case NodeOperation::AttributeRemove:
			{
			UINT id=0;
			size+=Dwarf::ReadUnsigned(stream, &id);
			Handle<String> key;
			if(id==0)
				{
				key=reader.ReadString(&size);
				}
			else
				{
				auto const& attr=node->m_Attributes.get_at(id-1);
				key=attr.get_key();
				}
			node->RemoveAttributeInternal(key);
			break;
			}
		case NodeOperation::AttributeSet:
			{
			UINT id=0;
			size+=Dwarf::ReadUnsigned(stream, &id);
			Handle<String> key;
			if(id==0)
				{
				key=reader.ReadString(&size);
				}
			else
				{
				auto const& attr=node->m_Attributes.get_at(id-1);
				key=attr.get_key();
				}
			auto value=reader.ReadString(&size);
			node->SetAttributeInternal(key, value);
			break;
			}
		case NodeOperation::ChildAppend:
			{
			auto child=Node::Create(m_Database);
			size+=child->ReadFromPage(stream);
			child->m_Parent=node;
			node->m_Children.append(child);
			break;
			}
		case NodeOperation::ChildInsert:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			auto child=Node::Create(m_Database);
			size+=child->ReadFromPage(stream);
			child->m_Parent=node;
			node->m_Children.insert_at(pos, child);
			break;
			}
		case NodeOperation::ChildRemove:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			node->m_Children.remove_at(pos);
			break;
			}
		case NodeOperation::None:
			{
			continue;
			}
		case NodeOperation::TargetSet:
			{
			node=this;
			UINT id=0;
			size+=Dwarf::ReadUnsigned(stream, &id);
			while(id!=0)
				{
				auto child=node->GetChildAt(id-1);
				node=child.As<Node>();
				size+=Dwarf::ReadUnsigned(stream, &id);
				}
			break;
			}
		case NodeOperation::ValueSet:
			{
			m_Value=reader.ReadString(&size);
			break;
			}
		}
	}
return size;
}

VOID Node::WriteToBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto block=Block::Create(volume);
block->Seek(block_id, 0);
SIZE_T size=0;
size+=block->Write(&NODE_ID, sizeof(UINT));
auto skip_bits=block->CreateSkipBits();
size+=skip_bits->WriteToStream(block);
size+=WriteToPage(block);
if(size%2)
	{
	BYTE zero=0;
	size+=block->Write(&zero, 1);
	}
block->Flush();
m_BlockId=block_id;
m_BlockPosition=(UINT)size;
}

UINT Node::WriteToPage(OutputStream* stream)
{
if(!m_Tag)
	throw InvalidArgumentException();
StreamWriter writer(stream);
SIZE_T size=0;
NodeOperation op=NodeOperation::NodeBegin;
size+=writer.Write(&op, sizeof(NodeOperation));
size+=writer.WriteString(m_Tag);
for(auto const& it: m_Attributes)
	{
	op=NodeOperation::AttributeSet;
	size+=writer.Write(&op, sizeof(NodeOperation));
	size+=Dwarf::WriteUnsigned(stream, 0U);
	size+=writer.WriteString(it.get_key());
	size+=writer.WriteString(it.get_value());
	}
if(m_Value)
	{
	op=NodeOperation::ValueSet;
	size+=writer.Write(&op, sizeof(NodeOperation));
	size+=writer.WriteString(m_Value);
	if(size%2)
		size+=writer.PrintChar('\0');
	return size;
	}
for(auto const& it: m_Children)
	{
	op=NodeOperation::ChildAppend;
	size+=writer.Write(&op, sizeof(NodeOperation));
	auto child=it.As<Node>();
	size+=child->WriteToPage(stream);
	}
if(size%2)
	size+=writer.PrintChar('\0');
return size;
}

}}