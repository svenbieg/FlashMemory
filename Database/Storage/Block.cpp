//===========
// Block.cpp
//===========

#include "Block.h"


//=======
// Using
//=======

#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Storage {


//==================
// Con-/Destructors
//==================

Handle<Block> Block::Create(Volume* volume, UINT id)
{
return Object::Create<Block>(volume, id);
}


//========
// Common
//========

VOID Block::Erase()
{
m_Volume->Erase(m_Id);
}

UINT Block::GetPosition()const
{
UINT pos=m_PageId*m_PageSize;
pos+=m_Page->GetPosition();
return pos;
}

VOID Block::Skip()
{
throw NotImplementedException();
}


//==============
// Input-Stream
//==============

SIZE_T Block::Available()
{
return m_Page->Available();
}

SIZE_T Block::Read(VOID* buf, SIZE_T size)
{
BYTE* dst=(BYTE*)buf;
SIZE_T read=0;
while(read<size)
	{
	SIZE_T available=m_Page->Available();
	if(!available)
		{
		if(++m_PageId>=m_PageCount)
			throw OutOfRangeException();
		m_Volume->ReadPage(m_Id, m_PageId, m_Page);
		available=m_Page->Available();
		if(!available)
			throw OutOfRangeException();
		}
	SIZE_T copy=TypeHelper::Min(size-read, available);
	read+=m_Page->Read(&dst[read], copy);
	}
return read;
}


//===============
// Output-Stream
//===============

VOID Block::Flush()
{
throw NotImplementedException();
}

SIZE_T Block::Write(VOID const* buf, SIZE_T size)
{
throw NotImplementedException();
return 0;
}


//==========================
// Con-/Destructors Private
//==========================

Block::Block(Volume* volume, UINT id):
m_Id(id),
m_PageCount(volume->GetBlockSize()/volume->GetPageSize()),
m_PageId(0),
m_PageSize(volume->GetPageSize()),
m_Size(volume->GetBlockSize()),
m_Volume(volume),
z_SkipBits(this)
{
m_Page=Page::Create(m_Volume);
m_Volume->ReadPage(m_Id, 0, m_Page);
}

}