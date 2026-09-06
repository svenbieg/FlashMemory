//===============
// MT29Flash.cpp
//===============

#include "MT29Flash.h"


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
	namespace Flash {


//===========
// Device-Id
//===========

const BYTE MICRON_ID=0x2C;

enum class Model: BYTE
{
MT29F1G01ABAFD=0x14
};


//==========
// Commands
//==========

const BYTE CMD_ERASE_BLOCK			=0xD8;
const BYTE CMD_GET_FEATURE			=0x0F;
const BYTE CMD_PROGRAM_EXEC			=0x10;
const BYTE CMD_PROGRAM_LOAD			=0x02;
const BYTE CMD_PROGRAM_LOAD_RND		=0x84;
const BYTE CMD_READ_CACHE			=0x03;
const BYTE CMD_READ_CACHE_LAST		=0x3F;
const BYTE CMD_READ_CACHE_RND		=0x30;
const BYTE CMD_READ_ID				=0x9F;
const BYTE CMD_READ_PAGE			=0x13;
const BYTE CMD_RESET				=0xFF;
const BYTE CMD_SET_FEATURE			=0x1F;
const BYTE CMD_WRITE_DISABLE		=0x04;
const BYTE CMD_WRITE_ENABLE			=0x06;


//==========
// Features
//==========

const BYTE FEAT_CONFIG		=0xB0;
const BYTE FEAT_LOCK		=0xA0;
const BYTE FEAT_STATUS		=0xC0;


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

VOID MT29Flash::Erase(UINT block)
{
WriteEnable();
BYTE tx[4];
tx[0]=CMD_ERASE_BLOCK;
tx[1]=(block>>16)&0xFF;
tx[2]=(block>>8)&0xFF;
tx[3]=block&0xFF;
m_SpiHost->SpiBegin(4, 0);
m_SpiHost->SpiWrite(tx, 4);
m_SpiHost->SpiEnd();
assert(BitHelper::Get(GetFeature(FEAT_STATUS), STATUS_OIP));
BYTE status=Wait(STATUS_OIP, 0);
WriteDisable();
if(BitHelper::Get(status, STATUS_EFAIL))
	throw ErrorException();
}

UINT MT29Flash::GetBlockSize()
{
return m_BlockSize;
}

WORD MT29Flash::GetPageSize(WORD* spare_ptr)
{
if(spare_ptr)
	*spare_ptr=m_PageSpare;
return m_PageSize;
}

UINT64 MT29Flash::GetSize()
{
return m_Size;
}

Handle<Page> MT29Flash::ReadPage(UINT block_id, WORD page_id)
{
UINT addr=block_id*m_PageCount+page_id;
BYTE tx[4];
tx[0]=CMD_READ_PAGE;
tx[1]=(addr>>16)&0xFF;
tx[2]=(addr>>8)&0xFF;
tx[3]=addr&0xFF;
m_SpiHost->SpiBegin(4, 0);
m_SpiHost->SpiWrite(tx, 4);
m_SpiHost->SpiEnd();
auto page=Page::Create(this);
auto buf=page->Begin();
Wait(STATUS_OIP, 0);
tx[0]=CMD_READ_CACHE;
tx[1]=0;
tx[2]=0;
tx[3]=0;
m_SpiHost->SpiBegin(4, m_PageTotal);
m_SpiHost->SpiWrite(tx, 4);
m_SpiHost->SpiRead(buf, m_PageTotal);
m_SpiHost->SpiEnd();
if(page_id==0)
	{
	if(buf[m_PageSize]==0)
		throw ErrorException();
	}
return page;
}

VOID MT29Flash::Write(UINT block, WORD page, WORD pos, VOID const* buf, WORD size)
{
WriteEnable();
BYTE tx[4];
tx[0]=CMD_PROGRAM_LOAD;
tx[1]=(pos>>8)&0xFF;
tx[2]=pos&0xFF;
m_SpiHost->SpiBegin(3+size, 0);
m_SpiHost->SpiWrite(tx, 3);
m_SpiHost->SpiWrite(buf, size);
m_SpiHost->SpiEnd();
UINT addr=block*m_PageCount+page;
tx[0]=CMD_PROGRAM_EXEC;
tx[1]=(addr>>16)&0xFF;
tx[2]=(addr>>8)&0xFF;
tx[3]=addr&0xFF;
m_SpiHost->SpiBegin(4, 0);
m_SpiHost->SpiWrite(tx, 4);
m_SpiHost->SpiEnd();
//assert(BitHelper::Get(GetFeature(FEAT_STATUS), STATUS_OIP));
BYTE status=Wait(STATUS_OIP, 0);
WriteDisable();
if(BitHelper::Get(status, STATUS_PFAIL))
	throw ErrorException();
}


//============================
// Con-/Destructors Protected
//============================

MT29Flash::MT29Flash(SpiHost* spi_host):
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
	case Model::MT29F1G01ABAFD:
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
SetFeature(FEAT_LOCK, 0);
SetFeature(FEAT_CONFIG, 0);
//BYTE lock=GetFeature(FEAT_LOCK);
//BYTE config=GetFeature(FEAT_CONFIG);
//return;
}


//================
// Common Private
//================

BYTE MT29Flash::GetFeature(BYTE feature)
{
BYTE tx[2]={ CMD_GET_FEATURE, feature };
BYTE rx[1];
m_SpiHost->SpiBegin(2, 1);
m_SpiHost->SpiWrite(tx, 2);
m_SpiHost->SpiRead(rx, 1);
m_SpiHost->SpiEnd();
return rx[0];
}

WORD MT29Flash::ReadId()
{
BYTE tx[2]={ CMD_READ_ID, 0 };
WORD id=0;
m_SpiHost->SpiBegin(2, 2);
m_SpiHost->SpiWrite(tx, 2);
m_SpiHost->SpiRead(&id, 2);
m_SpiHost->SpiEnd();
return id;
}

VOID MT29Flash::Reset()
{
BYTE tx[1]={ CMD_RESET };
m_SpiHost->SpiBegin(1, 0);
m_SpiHost->SpiWrite(tx, 1);
m_SpiHost->SpiEnd();
}

VOID MT29Flash::SetFeature(BYTE feature, BYTE value)
{
BYTE tx[3]={ CMD_SET_FEATURE, feature, value };
m_SpiHost->SpiBegin(3, 0);
m_SpiHost->SpiWrite(tx, 3);
m_SpiHost->SpiEnd();
}

BYTE MT29Flash::Wait(BYTE mask, BYTE value, UINT ms)
{
UINT64 timeout=SystemTimer::GetTickCount()+ms;
BYTE status=0;
while(1)
	{
	status=GetFeature(FEAT_STATUS);
	if(BitHelper::Get(status, mask)==value)
		break;
	if(SystemTimer::GetTickCount()>=timeout)
		throw TimeoutException();
	}
return status;
}

VOID MT29Flash::WriteDisable()
{
BYTE tx[1]={ CMD_WRITE_DISABLE };
m_SpiHost->SpiBegin(1, 0);
m_SpiHost->SpiWrite(tx, 1);
m_SpiHost->SpiEnd();
}

VOID MT29Flash::WriteEnable()
{
BYTE tx[1]={ CMD_WRITE_ENABLE };
m_SpiHost->SpiBegin(1, 0);
m_SpiHost->SpiWrite(tx, 1);
m_SpiHost->SpiEnd();
}

}}