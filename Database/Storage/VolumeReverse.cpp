//===================
// VolumeReverse.cpp
//===================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

// User-data is stored backwards on embedded OTA devices.

#include "VolumeReverse.h"


//===========
// Namespace
//===========

namespace Storage {


//========
// Volume
//========

VOID VolumeReverse::Erase(UINT64 offset, UINT size)
{
UINT64 redir=m_Size-offset-size;
m_Volume->Erase(redir, size);
}

UINT VolumeReverse::GetBlockSize()
{
return m_BlockSize;
}

UINT VolumeReverse::GetPageSize()
{
return m_PageSize;
}

UINT64 VolumeReverse::GetSize()
{
return m_Size;
}

VOID VolumeReverse::Read(UINT64 offset, VOID* buf, SIZE_T size)
{
UINT64 redir=Redirect(offset, size);
m_Volume->Read(redir, buf, size);
}

VOID VolumeReverse::SetSize(UINT64 size)
{
if(size>m_Size)
	throw OutOfMemoryException();
}

VOID VolumeReverse::Write(UINT64 offset, VOID const* buf, SIZE_T size)
{
UINT64 redir=Redirect(offset, size);
m_Volume->Write(redir, buf, size);
}


//==========================
// Con-/Destructors Private
//==========================

VolumeReverse::VolumeReverse(Volume* volume):
m_BlockSize(volume->GetBlockSize()),
m_PageSize(volume->GetPageSize()),
m_Size(volume->GetSize()),
m_Volume(volume)
{
m_BlockCount=m_Size/m_BlockSize;
}


//================
// Common Private
//================

UINT64 VolumeReverse::Redirect(UINT64 offset, SIZE_T size)
{
UINT block=offset/m_BlockSize;
UINT block_pos=offset%m_BlockSize;
SIZE_T copy=TypeHelper::AlignUp(size, m_BlockSize);
UINT block_count=copy/m_BlockSize;
block=m_BlockCount-block-block_count;
return block*m_BlockSize+block_pos;
}

}