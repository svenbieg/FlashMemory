//===========
// Block.cpp
//===========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Block.h"


//=======
// Using
//=======

#include "MemoryHelper.h"
#include <cassert>


//===========
// Namespace
//===========

namespace Storage {


//==================
// Con-/Destructors
//==================

Handle<Block> Block::Create(Volume* volume)
{
UINT page_size=volume->GetPageSize();
return Object::CreateEx<Block, Volume*>(page_size, sizeof(SIZE_T), volume);
}


//========
// Common
//========

Handle<Block::SkipBitArray> Block::CreateSkipBits(UINT skip)
{
UINT page_count=m_Size/m_PageSize;
auto skip_bits=SkipBitArray::Create(page_count/32);
skip_bits->Fill(-1);
if(skip==0)
	return skip_bits;
UINT pos=0;
for(auto it=skip_bits->First(); it->HasCurrent(); it->MoveNext())
	{
	if(pos>=skip)
		break;
	UINT mask=-1;
	UINT shift=TypeHelper::Min(skip-pos, 32);
	mask>>=shift;
	mask<<=shift;
	it->SetCurrent(mask);
	pos+=32;
	}
return skip_bits;
}

VOID Block::Seek(UINT pos, BlockLimit limit)
{
SetLimit(pos, limit);
m_Position=pos;
m_Written=0;
UINT page=m_Position/m_PageSize;
if(m_Page!=page)
	m_Page=-1;
}

VOID Block::Seek(UINT block, UINT pos, BlockLimit limit)
{
UINT64 offset=(UINT64)block*m_Size;
if(offset==m_Offset)
	{
	Seek(pos, limit);
	return;
	}
SetLimit(pos, limit);
m_Offset=offset;
m_Page=-1;
m_Position=pos;
m_Written=0;
}

UINT Block::SkipPages(SkipBitArray* skip_bits)
{
UINT count=skip_bits->GetCount();
UINT page=0;
for(auto it=skip_bits->First(); it->HasCurrent(); it->MoveNext())
	{
	UINT bits=it->GetCurrent();
	if(bits)
		{
		page+=Cpu::CountTrailingZeros(bits);
		break;
		}
	page+=32;
	}
if(page==0)
	return 0;
UINT pos=page*m_PageSize;
Seek(pos);
return pos-m_Position;
}


//==============
// Input-Stream
//==============

SIZE_T Block::Available()
{
return m_Limit-m_Position;
}

SIZE_T Block::Read(VOID* buf, SIZE_T size)
{
BYTE* dst=(BYTE*)buf;
SIZE_T read=0;
while(read<size)
	{
	UINT page_pos=m_Position%m_PageSize;
	if(page_pos==0)
		m_Page=-1;
	if(m_Page==-1)
		{
		if(m_Position>=m_Limit)
			throw OutOfRangeException();
		m_Page=m_Position/m_PageSize;
		UINT64 offset=m_Offset+(m_Page*m_PageSize);
		m_Volume->Read(offset, m_Buffer, m_PageSize);
		}
	UINT available=m_PageSize-page_pos;
	SIZE_T copy=TypeHelper::Min(size-read, available);
	MemoryHelper::Copy(&dst[read], &m_Buffer[page_pos], copy);
	m_Position+=copy;
	read+=copy;
	}
return read;
}


//===============
// Output-Stream
//===============

VOID Block::Flush()
{
if(!m_Written)
	return;
if(m_Position==0)
	m_Volume->Erase(m_Offset, m_Size);
UINT page_pos=m_Position%m_PageSize;
m_Volume->Write(m_Offset+m_Position, &m_Buffer[page_pos], m_Written);
m_Position+=m_Written;
m_Written=0;
}

SIZE_T Block::Write(VOID const* buf, SIZE_T size)
{
BYTE* src=(BYTE*)buf;
SIZE_T written=0;
while(written<size)
	{
	UINT pos=m_Position+m_Written;
	UINT page_pos=pos%m_PageSize;
	if(page_pos==0)
		{
		Flush();
		if(pos>=m_Limit)
			throw OutOfRangeException();
		}
	UINT available=m_PageSize-page_pos;
	SIZE_T copy=TypeHelper::Min(size-written, available);
	MemoryHelper::Copy(&m_Buffer[page_pos], &src[written], copy);
	m_Written+=copy;
	written+=copy;
	}
return written;
}


//==========================
// Con-/Destructors Private
//==========================

Block::Block(BYTE* buf, SIZE_T size, Volume* vol):
RandomAccessStream(StreamFormat::UTF8),
m_Buffer(buf),
m_Limit(size),
m_Offset(0),
m_Page(-1),
m_PageSize(size),
m_Position(0),
m_Size(vol->GetBlockSize()),
m_Volume(vol),
m_Written(0)
{}


//================
// Common Private
//================

VOID Block::SetLimit(UINT pos, BlockLimit limit)
{
switch(limit)
	{
	case BlockLimit::Block:
		{
		m_Limit=m_Size;
		break;
		}
	case BlockLimit::Page:
		{
		UINT page=pos/m_PageSize;
		m_Limit=(page+1)*m_PageSize;
		assert(m_Limit<=m_Size);
		break;
		}
	}
}

}