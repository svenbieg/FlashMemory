//=================
// FlashMemory.cpp
//=================

#include "FlashMemory.h"


//=======
// Using
//=======

#include "Exception.h"
#include "StringClass.h"


//===========
// Namespace
//===========

namespace Storage {


//========
// Volume
//========

VOID FlashMemory::Erase(UINT64 offset, UINT size)
{
assert(size>0);
assert(size%BLOCK_SIZE==0);
OVERLAPPED it={ 0 };
it.Offset=TypeHelper::LowLong(offset);
it.OffsetHigh=TypeHelper::HighLong(offset);
UINT pos=0;
while(pos<size)
	{
	UINT written=0;
	if(!WriteFile(m_File, m_Erase, BLOCK_SIZE, (DWORD*)&written, &it))
		throw DeviceNotReadyException();
	if(written!=BLOCK_SIZE)
		throw DeviceNotReadyException();
	pos+=written;
	}
}

WORD FlashMemory::GetAlignment()
{
return 1;
}

UINT FlashMemory::GetBlockSize()
{
return BLOCK_SIZE;
}

UINT FlashMemory::GetPageSize()
{
return PAGE_SIZE;
}

UINT64 FlashMemory::GetSize()
{
return 0;
}

VOID FlashMemory::Read(UINT64 offset, VOID* buf, SIZE_T size)
{
OVERLAPPED it={ 0 };
it.Offset=TypeHelper::LowLong(offset);
it.OffsetHigh=TypeHelper::HighLong(offset);
SIZE_T pos=0;
while(pos<size)
	{
	UINT copy=TypeHelper::Min(0x10000000UL, size-pos);
	UINT read=0;
	if(!ReadFile(m_File, buf, copy, (DWORD*)&read, &it))
		throw DeviceNotReadyException();
	if(read!=copy)
		throw DeviceNotReadyException();
	pos+=read;
	}
}

VOID FlashMemory::SetSize(UINT64 size)
{
LARGE_INTEGER pos;
pos.QuadPart=size;
if(!SetFilePointerEx(m_File, pos, nullptr, FILE_BEGIN))
	throw OutOfMemoryException();
if(!SetEndOfFile(m_File))
	throw OutOfMemoryException();
}

VOID FlashMemory::Write(UINT64 offset, VOID const* buf, SIZE_T size)
{
OVERLAPPED it={ 0 };
it.Offset=TypeHelper::LowLong(offset);
it.OffsetHigh=TypeHelper::HighLong(offset);
SIZE_T pos=0;
while(pos<size)
	{
	UINT copy=TypeHelper::Min(0x10000000UL, size-pos);
	UINT written=0;
	if(!WriteFile(m_File, buf, copy, (DWORD*)&written, &it))
		throw DeviceNotReadyException();
	if(written!=copy)
		throw DeviceNotReadyException();
	pos+=written;
	}
}


//==========================
// Con-/Destructors Private
//==========================

FlashMemory::FlashMemory(Handle<String> path):
m_BlockSize(BLOCK_SIZE),
m_File(NULL)
{
SetLastError(0);
#ifdef _UNICODE
m_File=CreateFileW(path->Begin(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, NULL);
#else
m_File=CreateFileA(path->Begin(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, NULL);
#endif
if(m_File==INVALID_HANDLE_VALUE)
	m_File=NULL;
if(!m_File)
	{
	DWORD err=GetLastError();
	switch(err)
		{
		case ERROR_ACCESS_DENIED:
			throw AccessDeniedException();
		default:
			throw Exception();
		}
	}
MemoryHelper::Fill(m_Erase, BLOCK_SIZE, 0xFF);
}

}