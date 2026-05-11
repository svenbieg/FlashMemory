//==============
// SpiFlash.cpp
//==============

#include "SpiFlash.h"


//=======
// Using
//=======

#include "Concurrency/Task.h"

using namespace Concurrency;
using namespace Devices::Spi;


//===========
// Namespace
//===========

namespace Devices {
	namespace Onfi {


//==========
// Settings
//==========

const BYTE MICRON_ID=0x2C;

enum class Model: BYTE
{
M78A_1Gb=0x14
};


//==========
// Commands
//==========

const UINT8 CMD_ERASE_BLOCK			=0xD8;
const UINT8 CMD_GET_FEATURES		=0x0F;
const UINT8 CMD_PROGRAM_EXEC		=0x10;
const UINT8 CMD_PROGRAM_LOAD		=0x02;
const UINT8 CMD_PROGRAM_LOAD_RND	=0x84;
const UINT8 CMD_READ_CACHE			=0x03;
const UINT8 CMD_READ_CACHE_LAST		=0x3F;
const UINT8 CMD_READ_CACHE_RND		=0x30;
const UINT8 CMD_READ_ID				=0x9F;
const UINT8 CMD_READ_PAGE			=0x13;
const UINT8 CMD_RESET				=0xFF;
const UINT8 CMD_SET_FEATURES		=0x1F;


//========
// Status
//========

const UINT8 STATUS_CRBSY	=(1<<7);
const BITS8 STATUS_ECC		={ 0x7, 4 };
const UINT8 STATUS_PFAIL	=(1<<3);
const UINT8 STATUS_EFAIL	=(1<<2);
const UINT8 STATUS_WEL		=(1<<1);
const UINT8 STATUS_OIP		=(1<<0);

const UINT8 ECC_OK			=0;
const UINT8 ECC_1_TO_3		=1;
const UINT8 ECC_FAILED		=2;
const UINT8 ECC_4_TO_6		=3;
const UINT8 ECC_7_TO_8		=5;


//========
// Volume
//========

VOID SpiFlash::Erase(UINT64 offset, UINT size)
{
throw NotImplementedException();
}

WORD SpiFlash::GetAlignment()
{
return 1;
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


//============================
// Con-/Destructors Protected
//============================

SpiFlash::SpiFlash(SpiHost* spi_host):
m_BlockSize(0),
m_Id(0),
m_PageSize(0),
m_Size(0),
m_SpiHost(spi_host)
{
Reset();
m_Id=ReadId();
BYTE micron=TypeHelper::LowByte(m_Id);
if(micron!=MICRON_ID)
	throw DeviceNotReadyException();
auto model=(Model)TypeHelper::HighByte(m_Id);
switch(model)
	{
	case Model::M78A_1Gb:
		{
		m_BlockSize=64*2048;
		m_PageSize=2048;
		m_PageSpare=128;
		m_Size=1024*64*2048;
		break;
		}
	default:
		{
		throw NotImplementedException();
		}
	}
}


//================
// Common Private
//================

WORD SpiFlash::ReadId()
{
BYTE tx[2]={ CMD_READ_ID, 0 };
WORD id=0;
m_SpiHost->SpiBegin(2, 2);
m_SpiHost->SpiWrite(tx, 2);
m_SpiHost->SpiRead(&id, 2);
m_SpiHost->SpiEnd();
return id;
}

VOID SpiFlash::Reset()
{
BYTE tx[2]={ CMD_RESET, 0 };
m_SpiHost->SpiBegin(2, 0);
m_SpiHost->SpiWrite(tx, 2);
m_SpiHost->SpiEnd();
Task::Sleep(10);
}

}}