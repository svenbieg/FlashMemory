//==============
// SpiFlash.cpp
//==============

#include "SpiFlash.h"


//=======
// Using
//=======

#include "Concurrency/Task.h"
#include "Devices/Timers/SystemTimer.h"

using namespace Concurrency;
using namespace Devices::Spi;
using namespace Devices::Timers;
using namespace Storage;


//===========
// Namespace
//===========

namespace Devices {
	namespace Onfi {


//===========
// Device-Id
//===========

const BYTE MICRON_ID=0x2C;

enum class Model: BYTE
{
M78A_1Gb=0x14
};


//==========
// Commands
//==========

const BYTE CMD_ERASE_BLOCK			=0xD8;
const BYTE CMD_GET_FEATURES			=0x0F;
const BYTE CMD_PROGRAM_EXEC			=0x10;
const BYTE CMD_PROGRAM_LOAD			=0x02;
const BYTE CMD_PROGRAM_LOAD_RND		=0x84;
const BYTE CMD_READ_CACHE			=0x03;
const BYTE CMD_READ_CACHE_LAST		=0x3F;
const BYTE CMD_READ_CACHE_RND		=0x30;
const BYTE CMD_READ_ID				=0x9F;
const BYTE CMD_READ_PAGE			=0x13;
const BYTE CMD_RESET				=0xFF;
const BYTE CMD_SET_FEATURES			=0x1F;


//==========
// Features
//==========

const BYTE FEAT_CONFIG		=0xB0;
const BYTE FEAT_STATUS		=0xC0;


//===============
// Configuration
//===============

const BYTE CONFIG_ECC_EN	=(1<<4);


//========
// Status
//========

const BYTE STATUS_CRBSY		=(1<<7);
const BYTE STATUS_PFAIL		=(1<<3);
const BYTE STATUS_EFAIL		=(1<<2);
const BYTE STATUS_WEL		=(1<<1);
const BYTE STATUS_OIP		=(1<<0);


//========
// Volume
//========

VOID SpiFlash::Erase(UINT block)
{
throw NotImplementedException();
}

UINT SpiFlash::GetBlockSize()
{
return m_BlockSize;
}

WORD SpiFlash::GetPageSize(WORD* spare_ptr)
{
if(spare_ptr)
	*spare_ptr=m_PageSpare;
return m_PageSize;
}

UINT64 SpiFlash::GetSize()
{
return m_Size;
}

VOID SpiFlash::Read(UINT block, WORD page, Page* buf)
{
UINT addr=block*m_PageCount+page;
BYTE tx[4];
tx[0]=CMD_READ_PAGE;
tx[1]=(addr>>16)&0xFF;
tx[2]=(addr>>8)&0xFF;
tx[3]=addr&0xFF;
m_SpiHost->SpiBegin(4, 0);
m_SpiHost->SpiWrite(tx, 4);
m_SpiHost->SpiEnd();
UINT64 timeout=SystemTimer::Microseconds64()+100;
while(1)
	{
	BYTE status=GetFeatures(FEAT_STATUS);
	if(!BitHelper::Get(status, STATUS_OIP))
		break;
	if(SystemTimer::Microseconds64()>=timeout)
		throw TimeoutException();
	}
auto dst=buf->Begin();
tx[0]=CMD_READ_CACHE;
tx[1]=0;
tx[2]=0;
tx[3]=0;
m_SpiHost->SpiBegin(4, m_PageTotal);
m_SpiHost->SpiWrite(tx, 4);
m_SpiHost->SpiRead(dst, m_PageTotal);
m_SpiHost->SpiEnd();
if(page==0)
	{
	if(dst[m_PageSize]==0)
		throw ErrorException();
	}
}

VOID SpiFlash::Write(UINT block, WORD page, WORD pos, VOID const* buf, WORD size)
{
throw NotImplementedException();
}


//============================
// Con-/Destructors Protected
//============================

SpiFlash::SpiFlash(SpiHost* spi_host):
m_BlockSize(0),
m_Id(0),
m_PageCount(0),
m_PageSize(0),
m_Size(0),
m_SpiHost(spi_host)
{
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
		m_PageCount=64;
		m_PageSize=2048;
		m_PageSpare=128;
		m_PageTotal=2048+128;
		m_Size=1024*64*2048;
		break;
		}
	default:
		{
		throw NotImplementedException();
		}
	}
BYTE config=GetFeatures(FEAT_CONFIG);
BitHelper::Clear(config, CONFIG_ECC_EN);
SetFeatures(FEAT_CONFIG, config);
}


//================
// Common Private
//================

BYTE SpiFlash::GetFeatures(BYTE feature)
{
BYTE tx[2]={ CMD_GET_FEATURES, feature };
BYTE rx[1];
m_SpiHost->SpiBegin(2, 1);
m_SpiHost->SpiWrite(tx, 2);
m_SpiHost->SpiRead(rx, 1);
m_SpiHost->SpiEnd();
return rx[0];
}

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

VOID SpiFlash::SetFeatures(BYTE feature, BYTE value)
{
BYTE tx[3]={ CMD_GET_FEATURES, feature, value };
m_SpiHost->SpiBegin(3, 0);
m_SpiHost->SpiWrite(tx, 3);
m_SpiHost->SpiEnd();
}

}}