//===============
// Redirectory.h
//===============

#pragma once


//=======
// Using
//=======

#include "Collections/index.hpp"
#include "Storage/Database/Entry.h"
#include "Storage/Streams/InputStream.h"
#include "Storage/Streams/OutputStream.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//======================
// Forward-Declarations
//======================

class Database;


//=============
// Redirectory
//=============

class Redirectory: public Object
{
public:
	// Using
	using InputStream=Storage::Streams::InputStream;
	using OutputStream=Storage::Streams::OutputStream;

	// Friends
	friend Database;
	friend Object;

private:
	// Settings
	static const UINT REDIR_ID=ENTRY_ID('RDIR');

	// Con-/Destructors
	Redirectory(Database* Database);
	Redirectory(Database* Database, UINT Block);
	static inline Handle<Redirectory> Create(Database* Database)
		{
		return Object::Create<Redirectory>(Database);
		}
	static inline Handle<Redirectory> Create(Database* Database, UINT Block)
		{
		return Object::Create<Redirectory>(Database, Block);
		}

	// Common
	WORD ReadFromStream(InputStream* Stream);
	Collections::index<UINT, BYTE, 8> m_Blocks;
	Database* m_Database;
	UINT m_Size;
};

}}