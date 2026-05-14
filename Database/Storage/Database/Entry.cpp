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
#include "Storage/Encoding/Dwarf.h"

using namespace Concurrency;
using namespace Storage::Encoding;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//========
// Common
//========

SIZE_T Entry::ReadFromStream(InputStream* stream)
{
SIZE_T size=0;
UINT id=0;
size+=stream->Read(&id, sizeof(UINT));
if(m_Id!=id)
	return 0;
size+=Dwarf::ReadUnsigned(stream, &m_EraseCount);
size+=Dwarf::ReadUnsigned(stream, &m_Parent);
return size;
}

SIZE_T Entry::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
size+=stream->Write(&m_Id, sizeof(UINT));
size+=Dwarf::WriteUnsigned(stream, m_EraseCount);
size+=Dwarf::WriteUnsigned(stream, m_Parent);
return size;
}


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

Entry::Entry(Database* database, UINT block, UINT id):
m_Block(block),
m_Database(database),
m_Id(id),
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
auto volume=m_Database->m_Volume;
auto block=Block::Create(volume, block_id);
WriteToStream(block);
block->Flush();
m_Block=block_id;
m_Size=block->GetPosition();
return m_Size;
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