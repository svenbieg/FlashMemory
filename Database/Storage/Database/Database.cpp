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
#include "Storage/Database/Map.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//==========================
// Con-/Destructors Private
//==========================

Database::Database(Volume* volume, FileCreateMode create):
m_Used(0),
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
		return;
		}
	case FileCreateMode::OpenAlways:
		{
		if(!header)
			{
			Initialize();
			return;
			}
		ValidateHeader(header);
		break;
		}
	case FileCreateMode::OpenExisting:
		{
		ValidateHeader(header);
		break;
		}
	}
auto redir=m_Header->GetAttribute("Redirect");
m_Redirection=Redirect::Create(this, redir->ToUInt());
}


//================
// Common Private
//================

VOID Database::Initialize()
{
m_Header=Node::Create(this, "Header");
m_Used=2;
m_Redirection=Redirect::Create(this);
UINT redir_id=m_Redirection->GetId();
m_Header->SetAttribute("Redirect", String::From(redir_id));
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

VOID Database::ValidateHeader(Node* header)
{
if(header->m_Id==0)
	{
	try
		{
		auto header1=Node::Create(this, 1);
		if(header1->m_Size!=header->m_Size)
			throw InvalidArgumentException();
		}
	catch(InvalidArgumentException)
		{
		header->WriteToBlock(1);
		header->WriteToBlock(0);
		}
	}
else
	{
	header->WriteToBlock(0);
	header->WriteToBlock(1);
	}
m_Header=header;
}

}}