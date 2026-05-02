//==========
// Node.cpp
//==========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Node.h"


//=======
// Using
//=======

#include "Concurrency/WriteLock.h"
#include "Devices/System/Cpu.h"
#include "Storage/Database/Database.h"
#include "Storage/Database/Editor.h"
#include "Storage/Database/NodeOperation.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"

using namespace Concurrency;
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


//============================
// Con-/Destructors Protected
//============================

Node::Node(Database* database, UINT block_id):
m_BlockId(-1),
m_BlockPosition(0),
m_Database(database),
m_Operation(nullptr)
{
ReadFromBlock(block_id);
}

Node::Node(Database* database, Handle<String> tag):
XmlNode(tag),
m_BlockId(-1),
m_BlockPosition(0),
m_Database(database),
m_Operation(nullptr)
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
size+=ReadFromStream(block);
size+=NodeOperation::ReadFromStream(this, block);
m_BlockId=block_id;
m_BlockPosition=(UINT)size;
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
size+=XmlNode::WriteToStream(block);
if(size%2)
	{
	BYTE zero=0;
	size+=block->Write(&zero, 1);
	}
block->Flush();
m_BlockId=block_id;
m_BlockPosition=(UINT)size;
}

}}