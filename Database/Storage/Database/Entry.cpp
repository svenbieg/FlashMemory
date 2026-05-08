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

Entry::Entry(Database* database, UINT block_id):
m_BlockId(block_id),
m_BlockPosition(0),
m_Database(database),
m_Id(0),
m_SkipBits(database->GetVolume()),
m_Update(nullptr)
{}


//==================
// Common Protected
//==================

SIZE_T Entry::Align(OutputStream* stream, SIZE_T size)
{
WORD align=m_Database->m_Alignment;
assert(align<=4);
if(size%align==0)
	return 0;
SIZE_T append=align-(size%align);
UINT zero=0;
OutputStream::Write(stream, &zero, append);
return append;
}

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

SIZE_T Entry::WriteToBlock(UINT block_id)
{
SIZE_T size=0;
auto block=Block::Create(m_Database, block_id);
size+=WriteEntry(block);
size+=Align(block, size);
size+=m_SkipBits.WriteBlockBits(block, 0);
size+=m_SkipBits.WritePageBits(block, 0);
block->Flush();
m_BlockId=block_id;
m_BlockPosition=block->GetPosition();
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
size=Align(stream, size);
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