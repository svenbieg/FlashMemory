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

Node::Node(Handle<String> tag):
XmlNode(tag),
m_BlockId(-1),
m_BlockPosition(0)
{}

Node::Node(Database* database, UINT block_id):
m_BlockId(-1),
m_BlockPosition(0)
{
ReadFromBlock(database, block_id);
}


//================
// Common Private
//================

Handle<Node::SkipBitArray> Node::CreateSkipBits(Volume* volume)
{
UINT block_size=volume->GetBlockSize();
UINT page_size=volume->GetPageSize();
UINT page_count=block_size/page_size;
return SkipBitArray::Create(page_count/32);
}

VOID Node::ReadFromBlock(Database* database, UINT block_id)
{
auto volume=database->GetVolume();
auto block=Block::Create(volume);
block->Seek(block_id, 0);
UINT id=0;
SIZE_T size=block->Read(&id, sizeof(UINT));
if(id!=NODE_ID)
	throw InvalidArgumentException();
auto skip_bits=CreateSkipBits(volume);
size+=skip_bits->ReadFromStream(block);
size+=SkipPages(block, skip_bits);
size+=ReadFromPage(block);
size+=ReadUpdates(block);
m_BlockId=block_id;
m_BlockPosition=(UINT)size;
}

UINT Node::ReadFromPage(Block* block)
{
StreamReader reader(block);
SIZE_T size=0;
NodeOperation op;
size+=block->Read(&op, sizeof(NodeOperation));
if(op!=NodeOperation::NodeBegin)
	throw InvalidArgumentException();
m_Tag=reader.ReadString(&size);
while(block->Available())
	{
	NodeOperation op;
	size+=block->Read(&op, sizeof(NodeOperation));
	if(op==NodeOperation::NodeEnd)
		break;
	switch(op)
		{
		case NodeOperation::AttributeSet:
			{
			UINT attr_id=0;
			size+=Dwarf::ReadUnsigned(block, &attr_id);
			if(attr_id!=0)
				throw InvalidArgumentException();
			auto key=reader.ReadString(&size);
			auto value=reader.ReadString(&size);
			m_Attributes.set(key, value);
			break;
			}
		case NodeOperation::ChildAppend:
			{
			auto child=Node::Create();
			size+=child->ReadFromPage(block);
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
return size;
}

UINT Node::ReadUpdates(Block* block)
{
StreamReader reader(block);
SIZE_T size=0;
Node* node=this;
while(block->Available())
	{
	NodeOperation op;
	size+=block->Read(&op, sizeof(NodeOperation));
	if(op==NodeOperation::Done)
		break;
	switch(op)
		{
		case NodeOperation::AttributeRemove:
			{
			UINT id=0;
			size+=Dwarf::ReadUnsigned(block, &id);
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
			size+=Dwarf::ReadUnsigned(block, &id);
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
			auto child=Node::Create();
			size+=child->ReadFromPage(block);
			child->m_Parent=node;
			node->m_Children.append(child);
			break;
			}
		case NodeOperation::ChildInsert:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(block, &pos);
			auto child=Node::Create();
			size+=child->ReadFromPage(block);
			child->m_Parent=node;
			node->m_Children.insert_at(pos, child);
			break;
			}
		case NodeOperation::ChildRemove:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(block, &pos);
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
			size+=Dwarf::ReadUnsigned(block, &id);
			while(id!=0)
				{
				auto child=node->GetChildAt(id-1);
				node=child.As<Node>();
				size+=Dwarf::ReadUnsigned(block, &id);
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

UINT Node::SkipPages(Block* block, SkipBitArray* skip_bits)
{
UINT count=skip_bits->GetCount();
UINT page=0;
for(auto it=skip_bits->First(); it->HasCurrent(); it->MoveNext())
	{
	UINT bits=it->GetCurrent();
	if(bits)
		{
		page+=Cpu::CountTrailingZeros(bits);
		break;
		}
	page+=32;
	}
if(page==0)
	return 0;
UINT block_pos=block->GetPosition();
UINT page_size=block->GetPageSize();
UINT pos=page*page_size;
block->Seek(pos);
return pos-block_pos;
}

VOID Node::WriteToBlock(Database* database, UINT block_id)
{
auto volume=database->GetVolume();
auto block=Block::Create(volume);
block->Seek(block_id, 0);
SIZE_T size=0;
size+=block->Write(&NODE_ID, sizeof(UINT));
auto skip_bits=CreateSkipBits(volume);
skip_bits->Fill(-1);
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

UINT Node::WriteToPage(Block* block)
{
if(!m_Tag)
	throw InvalidArgumentException();
StreamWriter writer(block);
SIZE_T size=0;
NodeOperation op=NodeOperation::NodeBegin;
size+=writer.Write(&op, sizeof(NodeOperation));
size+=writer.WriteString(m_Tag);
for(auto const& it: m_Attributes)
	{
	op=NodeOperation::AttributeSet;
	size+=writer.Write(&op, sizeof(NodeOperation));
	size+=Dwarf::WriteUnsigned(block, 0U);
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
	size+=child->WriteToPage(block);
	}
if(size%2)
	size+=writer.PrintChar('\0');
return size;
}

}}