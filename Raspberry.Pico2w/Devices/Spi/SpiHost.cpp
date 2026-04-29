//=============
// SpiHost.cpp
//=============

#include "SpiHost.h"


//=======
// Using
//=======

#include "Devices/System/System.h"
#include "Devices/Peripherals.h"

using namespace Concurrency;
using namespace Devices::Dma;
using namespace Devices::Gpio;
using namespace Devices::System;


//===========
// Namespace
//===========

namespace Devices {
	namespace Spi {


//=========
// Devices
//=========

typedef struct
{
SIZE_T BASE;
GpioPin RX;
GpioPin CS;
GpioPin SCK;
GpioPin TX;
DmaRequest DMA_TX;
DmaRequest DMA_RX;
ResetDevice RESET;
}SPI_DEVICE;

const SPI_DEVICE SPI_DEVICES[]=
	{
	{ SPI0_BASE, GpioPin::Gpio16, GpioPin::Gpio17, GpioPin::Gpio18, GpioPin::Gpio19, DmaRequest::Spi0Tx, DmaRequest::Spi0Rx, ResetDevice::Spi0 },
	{ SPI1_BASE, GpioPin::Gpio12, GpioPin::Gpio13, GpioPin::Gpio14, GpioPin::Gpio15, DmaRequest::Spi1Tx, DmaRequest::Spi1Rx, ResetDevice::Spi1 }
	};

const UINT SPI_COUNT=TypeHelper::ArraySize(SPI_DEVICES);

Mutex g_Mutex;
SpiHost* g_SpiHosts[SPI_COUNT]={ nullptr };


//===========
// Baud-Rate
//===========

typedef struct
{
BYTE PRESCAL; // 2..254 (%2)
BYTE DIVISOR; // 0..255 (+1)
}CLK_DIV;

const CLK_DIV CLK_DIVS[]=
	{
	{ 2, 0 } // 75M
	};


//===========
// Registers
//===========

typedef struct
{
RW32 CTRL0;
RW32 CTRL1;
RW32 DATA;
RO32 STATUS;
RW32 PRESCAL;
RW32 IRQMSC;
RO32 IRQS;
RO32 IRQMS;
RW32 IRQC;
RW32 DMACTRL;
}PL022_REGS;

const BITS CTRL0_SCR	={ 0xFF, 8 };
const BITS CTRL0_FRF	={ 0x3, 4 };
const BITS CTRL0_DSS	={ 0xF, 0 };

typedef enum
{
FRF_TI=1
}FRF;

typedef enum
{
DSS_4BIT=3,
DSS_8BIT=7,
DSS_16BIT=15
}DSS;

const UINT CTRL1_EN		=(1<<1);

const UINT STATUS_BUSY	=(1<<4);
const UINT STATUS_RF	=(1<<3);
const UINT STATUS_RNE	=(1<<2);
const UINT STATUS_TNF	=(1<<1);
const UINT STATUS_TE	=(1<<0);

const UINT DMACTRL_TXEN	=(1<<1);
const UINT DMACTRL_RXEN	=(1<<0);


//==================
// Con-/Destructors
//==================

Handle<SpiHost> SpiHost::Create(SpiDevice device, BaudRate baud)
{
WriteLock lock(g_Mutex);
UINT id=(UINT)device;
if(g_SpiHosts[id])
	throw AccessDeniedException();
auto spi_host=Object::Create<SpiHost>(device, baud);
g_SpiHosts[id]=spi_host;
return spi_host;
}

SpiHost::~SpiHost()
{
WriteLock lock(g_Mutex);
System::System::Disable(SPI_DEVICES[m_Id].RESET);
g_SpiHosts[m_Id]=nullptr;
}


//========
// Common
//========

VOID SpiHost::Read(UINT* buf, UINT count, UINT timeout)
{
auto spi=(PL022_REGS*)m_Device;
m_InputDma->BeginRead(m_InputRequest, &spi->DATA, buf, count);
m_InputDma->Wait(timeout);
}

VOID SpiHost::Write(UINT const* buf, UINT count, UINT timeout)
{
auto spi=(PL022_REGS*)m_Device;
m_OutputDma->BeginWrite(m_OutputRequest, &spi->DATA, buf, count);
m_OutputDma->Wait(timeout);
}


//==========================
// Con-/Destructors Private
//==========================

SpiHost::SpiHost(SpiDevice device, BaudRate baud):
m_Id((UINT)device)
{
m_Device=SPI_DEVICES[m_Id].BASE;
System::System::Enable(SPI_DEVICES[m_Id].RESET);
m_GpioHost=GpioHost::Create();
m_GpioHost->SetPinMode(SPI_DEVICES[m_Id].CS, GpioPinMode::Func1);
m_GpioHost->SetPinMode(SPI_DEVICES[m_Id].SCK, GpioPinMode::Func1);
m_GpioHost->SetPinMode(SPI_DEVICES[m_Id].RX, GpioPinMode::Func1);
m_GpioHost->SetPinMode(SPI_DEVICES[m_Id].TX, GpioPinMode::Func1);
m_InputDma=DmaChannel::Create();
m_InputDma->SetDataSize(DmaDataSize::Bits16);
m_InputRequest=SPI_DEVICES[m_Id].DMA_RX;
m_OutputDma=DmaChannel::Create();
m_OutputDma->SetDataSize(DmaDataSize::Bits16);
m_OutputRequest=SPI_DEVICES[m_Id].DMA_TX;
auto spi=(PL022_REGS*)SPI_DEVICES[m_Id].BASE;
UINT baud_id=(UINT)baud;
IoHelper::Set(spi->PRESCAL, CTRL0_SCR, CLK_DIVS[baud_id].PRESCAL);
IoHelper::Set(spi->CTRL0, CTRL0_SCR, CLK_DIVS[baud_id].DIVISOR);
IoHelper::Set(spi->CTRL1, CTRL1_EN);
}

}}