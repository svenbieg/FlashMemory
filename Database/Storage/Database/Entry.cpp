//===========
// Entry.cpp
//===========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Entry.h"


//=======
// Using
//=======

#include "Storage/Streams/StreamReader.h"
#include "MemoryHelper.h"

using namespace Storage::Streams;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//==============
// Input-Stream
//==============

SIZE_T Entry::Available()
{
return m_Limit-m_Position;
}

SIZE_T Entry::Read(VOID* buf, SIZE_T size)
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
	SIZE_T copy=TypeHelper::Min(available, size-read);
	MemoryHelper::Copy(&dst[read], &m_Buffer[page_pos], copy);
	m_Position+=copy;
	read+=copy;
	}
return read;
}


//===============
// Output-Stream
//===============

VOID Entry::Flush()
{
if(!m_Written)
	return;
UINT page_pos=m_Position%m_PageSize;
m_Volume->Write(m_Offset+m_Position, &m_Buffer[page_pos], m_Written);
m_Position+=m_Written;
m_Written=0;
}

SIZE_T Entry::Write(VOID const* buf, SIZE_T size)
{
BYTE* src=(BYTE*)buf;
SIZE_T written=0;
while(written<size)
	{
	UINT pos=m_Position+m_Written;
	if(pos>=m_Limit)
		throw OutOfRangeException();
	UINT page_pos=pos%m_PageSize;
	UINT available=m_PageSize-page_pos;
	SIZE_T copy=TypeHelper::Min(available, size-written);
	MemoryHelper::Copy(&m_Buffer[page_pos], &src[written], copy);
	m_Written+=copy;
	written+=copy;
	}
return written;
}


//==========================
// Con-/Destructors Private
//==========================

Entry::Entry(BYTE* buf, SIZE_T size, Volume* volume, UINT block, UINT id, FileCreateMode create):
m_BlockSize(volume->GetBlockSize()),
m_Buffer(buf),
m_Limit(size),
m_Offset(block*m_BlockSize),
m_Page(-1),
m_PageSize(size),
m_Position(0),
m_Volume(volume),
m_Written(0)
{
StreamReader reader(this);
if(create!=FileCreateMode::CreateAlways)
	{
	UINT block_id=0;
	reader.Read(&block_id, sizeof(UINT));
	if(block_id==id)
		{
		if(create==FileCreateMode::CreateNew)
			throw AlreadyExistsException();
		auto skip_bits=CreateSkipBits();
		skip_bits->ReadFromStream(this);
		SkipPages(skip_bits);
		return;
		}
	if(create==FileCreateMode::OpenExisting)
		throw NotFoundException();
	}
m_Volume->Erase(m_Offset, m_BlockSize);
m_Volume->Write(m_Offset, &id, sizeof(UINT));
MemoryHelper::Copy(m_Buffer, &id, sizeof(UINT));
UINT skip_bits=m_BlockSize/m_PageSize/8;
MemoryHelper::Fill(&m_Buffer[sizeof(UINT)], skip_bits, 0xFF);
m_Position=sizeof(UINT)+skip_bits;
}

Handle<Entry> Entry::Create(Volume* volume, UINT block, UINT id, FileCreateMode create)
{
UINT page_size=volume->GetPageSize();
return Object::CreateEx<Entry, Volume*>(page_size, sizeof(SIZE_T), volume, block, id, create);
}


//================
// Common Private
//================

Handle<Entry::SkipBitArray> Entry::CreateSkipBits(UINT skip)
{
UINT page_count=m_BlockSize/m_PageSize;
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

VOID Entry::SkipPages(SkipBitArray* skip_bits)
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
if(page>0)
	{
	m_Page=-1;
	m_Position=page*m_PageSize;
	m_Limit=m_Position+m_PageSize;
	}
}

}}