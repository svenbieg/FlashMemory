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


//==============
// Input-Stream
//==============

SIZE_T Page::Available()
{
return m_Size-m_Position;
}

SIZE_T Page::Read(VOID* buf, SIZE_T size)
{
WORD copy=TypeHelper::Min(size, m_Size-m_Position);
MemoryHelper::Copy(buf, &m_Buffer[m_Position], copy);
m_Position+=copy;
return copy;
}


//===============
// Output-Stream
//===============

VOID Page::Flush()
{
}

SIZE_T Page::Write(VOID const* buf, SIZE_T size)
{
WORD copy=TypeHelper::Min(size, m_Size-m_Position);
MemoryHelper::Copy(&m_Buffer[m_Position], buf, copy);
m_Position+=copy;
return copy;
}


//==========================
// Con-/Destructors Private
//==========================

Page::Page(BYTE* buf, SIZE_T size, Volume* volume):
m_Buffer(buf),
m_Position(0),
m_Size(volume->GetPageSize())
{}

}