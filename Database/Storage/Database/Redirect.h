//============
// Redirect.h
//============

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


//==========
// Redirect
//==========

class Redirect: public Object
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
	static const UINT REDIR_SIZE=6;
	static const UINT REDIR_TYPE=ENTRY_TYPE('RDIR');

	// Con-/Destructors
	Redirect(Database* Database);
	Redirect(Database* Database, UINT Block);
	static inline Handle<Redirect> Create(Database* Database)
		{
		return Object::Create<Redirect>(Database);
		}
	static inline Handle<Redirect> Create(Database* Database, UINT Block)
		{
		return Object::Create<Redirect>(Database, Block);
		}

	// Common
	WORD ReadFromStream(InputStream* Stream);
	Collections::index<UINT, BYTE, 8> m_Blocks;
	Database* m_Database;
	UINT m_Size;
};

}}