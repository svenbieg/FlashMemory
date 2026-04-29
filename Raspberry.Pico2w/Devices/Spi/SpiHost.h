//===========
// SpiHost.h
//===========

#pragma once


//=======
// Using
//=======

#include "Devices/Dma/DmaChannel.h"
#include "Devices/Gpio/GpioHost.h"


//===========
// Namespace
//===========

namespace Devices {
	namespace Spi {


//=========
// Devices
//=========

enum class SpiDevice
{
Spi0,
Spi1
};


//===========
// Baud-Rate
//===========

enum class BaudRate: UINT
{
Baud75M
};


//==========
// SPI-Host
//==========

class SpiHost: public Object
{
public:
	// Using
	using DmaChannel=Devices::Dma::DmaChannel;
	using DmaRequest=Devices::Dma::DmaRequest;
	using GpioHost=Devices::Gpio::GpioHost;

	// Con-/Destructors
	static Handle<SpiHost> Create(SpiDevice Device=SpiDevice::Spi0, BaudRate Baud=BaudRate::Baud75M);
	~SpiHost();

	// Common
	VOID Read(UINT* Buffer, UINT Count, UINT Timeout=100);
	VOID Write(UINT const* Buffer, UINT Count, UINT Timeout=100);

private:
	// Con-/Destructors
	friend Object;
	SpiHost(SpiDevice Device, BaudRate Baud);

	// Common
	SIZE_T m_Device;
	Handle<GpioHost> m_GpioHost;
	UINT m_Id;
	Handle<DmaChannel> m_InputDma;
	DmaRequest m_InputRequest;
	Handle<DmaChannel> m_OutputDma;
	DmaRequest m_OutputRequest;
};

}}