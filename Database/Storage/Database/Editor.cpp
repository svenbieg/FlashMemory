//============
// Editor.cpp
//============

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Editor.h"


//=======
// Using
//=======

#include "Storage/Database/Database.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Block.h"

using namespace Storage::Encoding;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//==================
// Con-/Destructors
//==================

Editor::~Editor()
{
Cancel();
m_Database->m_Mutex.Unlock();
}


//========
// Common
//========

UINT Editor::AllocateBlock()
{
throw NotImplementedException();
return 0;
}

VOID Editor::Cancel()
{
throw NotImplementedException();
}

VOID Editor::Flush()
{
throw NotImplementedException();
}

VOID Editor::FreeBlock(UINT block)
{
throw NotImplementedException();
}


//==========================
// Con-/Destructors Private
//==========================

Editor::Editor(Database* database):
m_Database(database)
{
m_Database->m_Mutex.Lock();
}

}}