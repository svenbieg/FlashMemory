//==============
// Database.cpp
//==============

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Database.h"


//=======
// Using
//=======

#include "Storage/Database/Editor.h"
#include "Storage/Block.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//========
// Common
//========

Handle<Editor> Database::Edit()
{
return Editor::Create(this);
}


//==========================
// Con-/Destructors Private
//==========================

Database::Database(Volume* volume, FileCreateMode create):
m_Volume(volume)
{
auto header=ReadHeader();
switch(create)
	{
	case FileCreateMode::CreateNew:
		{
		if(header)
			throw AlreadyExistsException();
		break;
		}
	case FileCreateMode::OpenExisting:
		{
		if(!header)
			throw NotFoundException();
		break;
		}
	}

}


//================
// Common Private
//================

Handle<Node> Database::ReadHeader()
{
auto header=Node::Create("Header");
auto block=Block::Create(m_Volume);
block->Seek(0, 0);
SIZE_T header_size=0;
try
	{
	header_size=header->ReadFromStream(block);
	}
catch(InvalidArgumentException)
	{
	header_size=0;
	}
if(header_size==0)
	{
	try
		{
		block->Seek(1, 0);
		header_size=header->ReadFromStream(block);
		}
	catch(InvalidArgumentException)
		{
		header_size=0;
		}
	}
if(header_size==0)
	return nullptr;
return header;
}

}}