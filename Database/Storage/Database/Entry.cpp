//===========
// Entry.cpp
//===========

#include "Entry.h"


//=======
// Using
//=======

#include "Concurrency/WriteLock.h"
#include "Storage/Database/Database.h"

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
m_Database(database)
{}


//==================
// Common Protected
//==================

UINT Entry::Release()noexcept
{
if(m_Block==-1)
	return Object::Release();
WriteLock lock(m_Database->m_Mutex);
UINT ref_count=Cpu::InterlockedDecrement(&m_ReferenceCount);
if(ref_count==0)
	{
	m_Database->m_Entries.remove(m_Block);
	delete this;
	}
return ref_count;
}

}}