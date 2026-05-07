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

Entry::Entry(Database* database, UINT block):
m_Block(block),
m_BlockPosition(0),
m_Database(database),
m_Parent(nullptr)
{}

Entry::Entry(Entry* parent):
m_Block(-1),
m_BlockPosition(0),
m_Database(parent->m_Database),
m_Parent(parent)
{}


//==================
// Common Protected
//==================

VOID Entry::Invalidate(Editor* editor)
{
auto entry=this;
while(entry)
	{
	if(entry->m_Block!=-1)
		{
		editor->m_ChangedEntries.add(entry);
		break;
		}
	entry=entry->m_Parent;
	}
}

UINT Entry::Release()noexcept
{
WriteLock lock(m_Database->m_EntriesMutex);
UINT ref_count=Cpu::InterlockedDecrement(&m_ReferenceCount);
if(ref_count>0)
	return ref_count;
if(m_Block!=-1)
	m_Database->m_Entries.remove(m_Block);
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