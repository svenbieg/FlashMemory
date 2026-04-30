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
switch(create)
	{
	case FileCreateMode::CreateNew:
	case FileCreateMode::CreateAlways:
		{
		Initialize();
		break;
		}
	case FileCreateMode::OpenExisting:
		{
		Validate();
		break;
		}
	}
}


//================
// Common Private
//================

VOID Database::Initialize()
{
m_Header=Node::Create(this);
m_Header->SetTag("Header");
m_Header->SetAttribute("Used", 2);
m_Header->WriteToBlock(0);
m_Header->WriteToBlock(1);
}

Handle<Node> Database::ReadHeader()
{
Handle<Node> header;
for(UINT block=0; block<2; block++)
	{
	try
		{
		header=Node::Create(this, block);
		}
	catch(InvalidArgumentException)
		{
		continue;
		}
	}
return header;
}

VOID Database::Validate()
{
throw NotImplementedException();
}

}}