//==================
// WearLeveling.cpp
//==================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database/wiki#wear-leveling

#include "WearLeveling.h"


//=======
// Using
//=======

#include "Storage/Buffer.h"


//===========
// Namespace
//===========

namespace Storage {


//==========
// Settings
//==========

const UINT REDIR_ID='RDIR';


//========
// Volume
//========

VOID WearLeveling::Erase(UINT64 offset, UINT size)
{
UINT64 redir=Redirect(offset);
try
	{
	m_Volume->Erase(redir, size);
	return;
	}
catch(ErrorException e)
	{
	redir=Spare(offset);
	}
m_Volume->Erase(redir, size);
}

UINT WearLeveling::GetBlockSize()
{
return m_BlockSize;
}

UINT WearLeveling::GetPageSize()
{
return m_PageSize;
}

UINT64 WearLeveling::GetSize()
{
return m_Size;
}

VOID WearLeveling::Read(UINT64 offset, VOID* buf, SIZE_T size)
{
offset=Redirect(offset);
m_Volume->Read(offset, buf, size);
}

VOID WearLeveling::SetSize(UINT64 size)
{
if(size>m_Size)
	throw OutOfMemoryException();
}

VOID WearLeveling::Write(UINT64 offset, VOID const* buf, SIZE_T size)
{
UINT64 redir=Redirect(offset);
try
	{
	m_Volume->Write(redir, buf, size);
	return;
	}
catch(ErrorException e)
	{
	UINT64 spare=Spare(offset);
	UINT block_pos=redir%m_BlockSize;
	if(block_pos>0)
		{
		UINT src=redir/m_BlockSize;
		UINT dst=spare/m_BlockSize;
		auto buf=Buffer::Create(m_PageSize);
		auto buf_ptr=buf->Begin();
		for(UINT pos=0; pos<block_pos; )
			{
			UINT copy=TypeHelper::Min(block_pos-pos, m_PageSize);
			m_Volume->Read(redir+pos, buf_ptr, copy);
			m_Volume->Write(spare+pos, buf_ptr, copy);
			pos+=copy;
			}
		}
	redir=spare;
	}
m_Volume->Write(redir, buf, size);
}


//==========================
// Con-/Destructors Private
//==========================

WearLeveling::WearLeveling(Volume* volume, FileCreateMode create, UINT spare):
m_BlockSize(volume->GetBlockSize()),
m_Count(1),
m_PageSize(volume->GetPageSize()),
m_Position(0),
m_Size(volume->GetSize()),
m_Spare(spare),
m_Volume(volume)
{
m_Size-=m_Spare*m_BlockSize;
auto buf=Buffer::Create(m_PageSize);
auto entries=(UINT*)buf->Begin();
m_Volume->Read(0, entries, m_PageSize);
if(create==FileCreateMode::CreateAlways)
	entries[0]=0;
if(entries[0]!=REDIR_ID)
	{
	if(create==FileCreateMode::OpenExisting)
		throw NotFoundException();
	entries[0]=REDIR_ID;
	m_Volume->Erase(0, m_BlockSize);
	m_Volume->Write(0, entries, sizeof(UINT));
	return;
	}
if(create==FileCreateMode::CreateNew)
	throw AlreadyExistsException();
UINT count=(m_PageSize/sizeof(UINT))-2;
for(UINT pos=1; pos<count; pos+=2)
	{
	if(entries[pos]==-1)
		return;
	if(entries[pos+1]==REDIR_ID)
		{
		m_Redirect.set(entries[pos], m_Count++);
		m_Position=(pos+2)*sizeof(UINT);
		continue;
		}
	if(entries[pos+1]==0)
		{
		m_Position=(pos+2)*sizeof(UINT);
		continue;
		}
	if(entries[pos+2]!=-1)
		throw ErrorException();
	entries[pos]=0;
	entries[pos+1]=0;
	m_Volume->Write(pos*sizeof(UINT), &entries[pos], 2*sizeof(UINT));
	m_Position=(pos+2)*sizeof(UINT);
	return;
	}
throw ErrorException();
}


//================
// Common Private
//================

UINT64 WearLeveling::Redirect(UINT64 offset)
{
offset+=m_Spare*m_BlockSize;
UINT block=offset/m_BlockSize;
UINT redir=0;
if(m_Redirect.try_get(block, &redir))
	{
	UINT block_pos=offset%m_BlockSize;
	offset=(UINT64)redir*m_BlockSize+block_pos;
	}
return offset;
}

UINT64 WearLeveling::Spare(UINT64 offset)
{
if(m_Count==m_Spare)
	throw AbortException();
offset+=m_Spare*m_BlockSize;
UINT block=offset/m_BlockSize;
UINT redir=m_Count++;
m_Redirect.set(block, redir);
UINT entry[2];
entry[0]=block;
entry[1]=REDIR_ID;
m_Volume->Write(m_Position, entry, 2*sizeof(UINT));
m_Position+=2*sizeof(UINT);
UINT block_pos=offset%m_BlockSize;
return (UINT64)redir*m_BlockSize+block_pos;
}

}