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


//==================
// Con-/Destructors
//==================

Entry::~Entry()
{
ClearUpdate();
}


//============================
// Con-/Destructors Protected
//============================

Entry::Entry(Database* database, UINT block):
m_Block(block),
m_Database(database),
m_Id(0),
m_Size(0),
m_Update(nullptr)
{}


//==================
// Common Protected
//==================

Handle<Volume> Entry::GetVolume()const
{
return m_Database->m_Volume;
}

VOID Entry::Invalidate(Editor* editor)
{
if(m_Block!=-1)
	editor->m_ChangedEntries.add(this);
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

SIZE_T Entry::WriteToBlock(UINT block_id)
{
SIZE_T size=0;
auto volume=m_Database->m_Volume;
auto block=Block::Create(volume, block_id);
size+=block->Write(&m_Id, sizeof(UINT));
size+=WriteEntry(block);
block->Flush();
m_Block=block_id;
return size;
}

SIZE_T Entry::WriteUpdates(OutputStream* stream)
{
SIZE_T size=0;
auto update=m_Update;
while(update)
	{
	size+=update->WriteToStream(stream);
	update=update->m_Next;
	}
return size;
}


//================
// Common Private
//================

VOID Entry::ClearUpdate()
{
while(m_Update)
	{
	auto update=m_Update;
	m_Update=update->m_Next;
	delete update;
	}
}

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