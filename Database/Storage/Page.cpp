//==========
// Page.cpp
//==========

#include "Page.h"


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

Handle<Page> Page::Create(Volume* volume)
{
WORD spare=0;
WORD size=volume->GetPageSize(&spare);
return Object::CreateEx<Page>(size+spare, sizeof(SIZE_T), volume);
}


//========
// Common
//========

WORD Page::GetPosition()const
{
return m_ErrorCorrection.m_Next;
}


//==============
// Input-Stream
//==============

SIZE_T Page::Available()
{
if(!m_Available)
	m_Available=m_ErrorCorrection.Available(this);
return m_Available;
}

SIZE_T Page::Read(VOID* buf, SIZE_T size)
{
auto dst=(BYTE*)buf;
SIZE_T pos=0;
while(pos<size)
	{
	if(!m_Available)
		m_Available=m_ErrorCorrection.Available(this);
	if(!m_Available)
		break;
	WORD copy=TypeHelper::Min(size-pos, m_Available);
	MemoryHelper::Copy(&dst[pos], &m_Buffer[m_Position], copy);
	m_Position+=copy;
	m_Available-=copy;
	pos+=copy;
	}
return pos;
}


//===============
// Output-Stream
//===============

VOID Page::Flush()
{
throw NotImplementedException();
}

SIZE_T Page::Write(VOID const* buf, SIZE_T size)
{
throw NotImplementedException();
return 0;
}


//==========================
// Con-/Destructors Private
//==========================

Page::Page(BYTE* buf, SIZE_T size, Volume* volume):
m_Available(0),
m_Position(0),
m_Size(volume->GetPageSize()),
z_SkipBits(this)
{}

}