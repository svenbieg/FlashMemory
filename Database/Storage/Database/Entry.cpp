//===========
// Entry.cpp
//===========

#include "Entry.h"


//=======
// Using
//=======

#include "Concurrency/WriteLock.h"
#include "Storage/Database/Database.h"
#include "Storage/Database/Editor.h"

using namespace Concurrency;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//============================
// Con-/Destructors Protected
//============================

Entry::Entry(Database* database, UINT block_id):
m_BlockId(block_id),
m_BlockPosition(0),
m_Database(database),
m_Id(0)
{
if(m_BlockId==-1)
	return;
m_Block=Block::Create(m_Database, m_BlockId);
m_Block->Read(&m_Id, sizeof(UINT));
}


//==================
// Common Protected
//==================

VOID Entry::Invalidate(Editor* editor)
{
if(m_BlockId!=-1)
	editor->m_ChangedEntries.add(this);
}

UINT Entry::Release()noexcept
{
WriteLock lock(m_Database->m_EntriesMutex);
UINT ref_count=Cpu::InterlockedDecrement(&m_ReferenceCount);
if(ref_count>0)
	return ref_count;
if(m_BlockId!=-1)
	m_Database->m_Entries.remove(m_BlockId);
delete this;
return 0;
}


//================
// Common Private
//================

Mutex& Entry::GetEntriesMutex(Database* database)
{
return database->m_EntriesMutex;
}

Entry* Entry::GetEntry(Database* database, UINT block)
{
Entry* entry=nullptr;
database->m_Entries.try_get(block, &entry);
return entry;
}

VOID Entry::SetEntry(Database* database, UINT block, Entry* entry)
{
database->m_Entries.set(block, entry);
}

}}