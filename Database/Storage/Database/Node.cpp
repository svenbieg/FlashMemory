//==========
// Node.cpp
//==========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Node.h"


//=======
// Using
//=======

#include "Storage/Database/Database.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Streams/StreamReader.h"
#include <bit>

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


//========
// Update
//========

enum class UpdateOperation: BYTE
{
None=0,
AttributeRemove,
AttributeSet,
ChildInsert,
ChildRemove,
TargetSet,
Done=255
};


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database):
m_BlockId(-1),
m_BlockPosition(0),
m_Database(database)
{
auto volume=m_Database->GetVolume();
UINT block_size=volume->GetBlockSize();
UINT page_size=volume->GetPageSize();
UINT skip_bits=block_size/page_size;
m_SkipBits=SkipBitArray::Create(skip_bits/32);
m_SkipBits->Fill(-1);
}

Node::Node(Database* database, UINT block_id):
Node(database)
{
m_BlockId=block_id;
auto volume=m_Database->GetVolume();
auto block=Block::Create(volume);
block->Seek(m_BlockId, 0);
m_BlockPosition=ReadFromBlock(block);
}


//================
// Common Private
//================

UINT Node::ReadFromBlock(Block* block)
{
UINT id=0;
SIZE_T size=block->Read(&id, sizeof(UINT));
if(id!=NODE_ID)
	throw InvalidArgumentException();
size+=m_SkipBits->ReadFromStream(block);
UINT page=SkipPages();
if(page>0)
	{
	UINT page_size=block->GetPageSize();
	UINT pos=page*page_size;
	block->Seek(pos);
	size=pos;
	}
try
	{
	size+=XmlNode::ReadFromStream(block);
	}
catch(OutOfRangeException)
	{
	throw InvalidArgumentException();
	}
size+=ReadUpdates(block);
return (UINT)size;
}

UINT Node::ReadUpdates(Block* block)
{
StreamReader reader(block);
SIZE_T size=0;
XmlNode* node=this;
while(block->Available())
	{
	UpdateOperation op;
	size+=block->Read(&op, 1);
	if(op==UpdateOperation::Done)
		break;
	if(op==UpdateOperation::None)
		continue;
	switch(op)
		{
		case UpdateOperation::AttributeRemove:
			{
			UINT attr_id=0;
			size+=Dwarf::ReadUnsigned(block, &attr_id);
			node->RemoveAttributeAt(attr_id, EventNotification::None);
			break;
			}
		case UpdateOperation::AttributeSet:
			{
			UINT attr_id=0;
			size+=Dwarf::ReadUnsigned(block, &attr_id);
			if(attr_id==0)
				{
				auto key=reader.ReadString(&size);
				auto value=reader.ReadString(&size);
				node->SetAttribute(key, value, EventNotification::None);
				}
			else
				{
				auto value=reader.ReadString(&size);
				node->SetAttributeAt(attr_id-1, value, EventNotification::None);
				}
			break;
			}
		case UpdateOperation::ChildInsert:
			{
			UINT pos=0;
			UINT count=0;
			size+=Dwarf::ReadUnsigned(block, &pos);
			size+=Dwarf::ReadUnsigned(block, &count);
			for(UINT u=0; u<count; u++)
				{
				auto child=XmlNode::Create();
				size+=child->ReadFromStream(block);
				node->InsertChildAt(pos++, child);
				}
			break;
			}
		case UpdateOperation::ChildRemove:
			{
			UINT child_id=0;
			size+=Dwarf::ReadUnsigned(block, &child_id);
			node->RemoveChildAt(child_id, EventNotification::None);
			}
		case UpdateOperation::TargetSet:
			{
			node=this;
			UINT child_id=0;
			size+=Dwarf::ReadUnsigned(block, &child_id);
			while(child_id!=0)
				{
				node=node->GetChildAt(child_id-1);
				size+=Dwarf::ReadUnsigned(block, &child_id);
				}
			break;
			}
		}
	}
return (UINT)size;
}

UINT Node::SkipPages()
{
UINT count=m_SkipBits->GetCount();
UINT pos=0;
for(auto it=m_SkipBits->First(); it->HasCurrent(); it->MoveNext())
	{
	UINT bits=it->GetCurrent();
	if(bits)
		{
		INT lsb=std::countr_zero(bits);
		return pos+lsb;
		}
	pos+=32;
	}
return 0;
}

UINT Node::WriteToBlock(UINT block_id)
{
auto volume=m_Database->GetVolume();
auto block=Block::Create(volume);
block->Seek(block_id, 0);
m_SkipBits->Fill(-1);
SIZE_T size=0;
size+=block->Write(&NODE_ID, sizeof(UINT));
size+=m_SkipBits->WriteToStream(block);
size+=XmlNode::WriteToStream(block, -1);
if(size%2)
	{
	BYTE zero=0;
	size+=block->Write(&zero, 1);
	}
block->Flush();
return (UINT)size;
}

}}