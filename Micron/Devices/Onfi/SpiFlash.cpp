//==============
// SpiFlash.cpp
//==============

#include "SpiFlash.h"


//=======
// Using
//=======

#include "Devices/Onfi/Onfi.h"


//===========
// Namespace
//===========

namespace Devices {
	namespace Onfi {


//========
// Common
//========

VOID SpiFlash::Reset()
{

}


//========
// Volume
//========

VOID SpiFlash::Erase(UINT64 offset, UINT size)
{
throw NotImplementedException();
}

WORD SpiFlash::GetAlignment()
{
return 2;
}

UINT SpiFlash::GetBlockSize()
{
return m_BlockSize;
}

UINT SpiFlash::GetPageSize()
{
return m_PageSize;
}

UINT64 SpiFlash::GetSize()
{
return m_Size;
}

VOID SpiFlash::Read(UINT64 offset, VOID* buf, SIZE_T size)
{
throw NotImplementedException();
}

VOID SpiFlash::SetSize(UINT64 size)
{
if(size>m_Size)
	throw OutOfMemoryException();
}

VOID SpiFlash::Write(UINT64 offset, VOID const* buf, SIZE_T size)
{
throw NotImplementedException();
}


//==========================
// Con-/Destructors Private
//==========================

SpiFlash::SpiFlash(SpiHost* spi_host):
m_BlockSize(0),
m_PageSize(0),
m_Size(0),
m_SpiHost(spi_host)
{}

}}