//=================
// Redirectory.cpp
//=================

#include "Redirectory.h"


//=======
// Using
//=======

#include "Storage/Database/Database.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//==========================
// Con-/Destructors Private
//==========================

Redirectory::Redirectory(Database* database):
m_Database(database),
m_Size(0)
{}

Redirectory::Redirectory(Database* database, UINT block_id):
m_Database(database),
m_Size(0)
{
auto volume=m_Database->m_Volume;
auto page=Page::Create(volume);
volume->ReadPage(block_id, 0, page);
m_Size=ReadFromStream(page);
}


//==================
// Common Protected
//==================

WORD Redirectory::ReadFromStream(InputStream* stream)
{
WORD size=0;
while(stream->Available())
	{
	UINT id=0;
	stream->Read(&id, sizeof(UINT));
	if(id==-1)
		break;
	size+=sizeof(UINT);
	if(id!=REDIR_ID)
		continue;
	UINT block[2];
	size+=stream->Read(&block[0], sizeof(UINT));
	size+=stream->Read(&block[1], sizeof(UINT));
	if(block[0]!=~block[1])
		continue;
	if(m_Blocks.remove(block[0]))
		continue;
	m_Blocks.add(block[0]);
	}
return size;
}

}}