//==============
// Redirect.cpp
//==============

#include "Redirect.h"


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

Redirect::Redirect(Database* database):
m_Database(database),
m_Id(database->m_Used),
m_Size(0)
{
auto volume=m_Database->m_Volume;
UINT block=m_Id;
UINT count=REDIR_COUNT+1;
for(UINT u=0; u<count; u++)
	{
	volume->Erase(block);
	block++;
	}
m_Database->m_Used+=count;
}

Redirect::Redirect(Database* database, UINT id):
m_Database(database),
m_Id(id),
m_Size(0)
{
auto volume=m_Database->m_Volume;
auto page=Page::Create(volume);
volume->ReadPage(m_Id, 0, page);
m_Size=ReadFromStream(page);
}


//==================
// Common Protected
//==================

WORD Redirect::ReadFromStream(InputStream* stream)
{
WORD size=0;
while(stream->Available())
	{
	UINT id=0;
	stream->Read(&id, sizeof(UINT));
	if(id==-1)
		break;
	size+=sizeof(UINT);
	if(id!=REDIR_TYPE)
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