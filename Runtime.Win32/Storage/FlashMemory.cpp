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

VOID FlashMemory::Erase(UINT block)
{
UINT64 offset=block*BLOCK_SIZE;
OVERLAPPED it={ 0 };
it.Offset=TypeHelper::LowLong(offset);
it.OffsetHigh=TypeHelper::HighLong(offset);
DWORD written=0;
if(!WriteFile(m_File, m_Erase, BLOCK_SIZE, &written, &it))
	throw ErrorException();
if(written!=BLOCK_SIZE)
	throw ErrorException();
}

UINT FlashMemory::GetBlockSize()
{
return BLOCK_SIZE;
}

WORD FlashMemory::GetPageSize(WORD* spare_ptr)
{
if(spare_ptr)
	*spare_ptr=0;
return PAGE_SIZE;
}

UINT64 FlashMemory::GetSize()
{
return 0;
}

VOID FlashMemory::ReadPage(UINT block, WORD id, Page* page)
{
UINT64 offset=block*BLOCK_SIZE;
offset+=id*PAGE_SIZE;
OVERLAPPED it={ 0 };
it.Offset=TypeHelper::LowLong(offset);
it.OffsetHigh=TypeHelper::HighLong(offset);
auto buf=page->Begin();
UINT copy=PAGE_SIZE;
DWORD read=0;
if(!ReadFile(m_File, buf, copy, &read, &it))
	throw ErrorException();
if(read!=copy)
	throw ErrorException();
}

BOOL FlashMemory::SetSize(UINT64 size)
{
LARGE_INTEGER pos;
pos.QuadPart=size;
if(!SetFilePointerEx(m_File, pos, nullptr, FILE_BEGIN))
	return false;
if(!SetEndOfFile(m_File))
	return false;
return true;
}

VOID FlashMemory::Write(UINT block, WORD page, WORD pos, VOID const* buf, WORD size)
{
UINT64 offset=block*BLOCK_SIZE;
offset+=page*PAGE_SIZE;
offset+=pos;
OVERLAPPED it={ 0 };
it.Offset=TypeHelper::LowLong(offset);
it.OffsetHigh=TypeHelper::HighLong(offset);
UINT copy=size-pos;
UINT written=0;
if(!WriteFile(m_File, buf, copy, (DWORD*)&written, &it))
	throw ErrorException();
if(written!=copy)
	throw ErrorException();
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