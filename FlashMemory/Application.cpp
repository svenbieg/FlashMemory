//=================
// Application.cpp
//=================

#include "Application.h"


//=======
// Using
//=======

#include "Concurrency/Scheduler.h"
#include "Devices/Timers/SystemTimer.h"
#include "UI/Console.h"
#include "StatusHelper.h"

using namespace Concurrency;
using namespace Devices::Timers;
using namespace FlashMemory;
using namespace Storage;
using namespace UI;

#ifdef _PICO2W

#include "Devices/Onfi/SpiFlash.h"
#include "Devices/Pio/SpiEmulator.h"

using namespace Devices::Gpio;
using namespace Devices::Onfi;
using namespace Devices::Pio;
using namespace Devices::Spi;

#else

#include "Storage/FlashMemory.h"

#endif


//=============
// Entry-Point
//=============

VOID Main()
{
auto app=Application::Create();
DispatchedQueue::Enter();
}


//===========
// Namespace
//===========

namespace FlashMemory {


//==========================
// Con-/Destructors Private
//==========================

Application::Application()
{
auto task=Task::Create(this, [this]()
	{
	#ifdef _PICO2W
	Console::Print("Initializing flash-chip...");
	SpiConfiguration config;
	config.Divisor=2;
	config.Mode=SpiMode::Bits8;
	config.PinChipSelect=GpioPin::Gpio17;
	config.PinClock=GpioPin::Gpio18;
	config.PinRx=GpioPin::Gpio16;
	config.PinTx=GpioPin::Gpio19;
	auto spi_host=SpiEmulator::Create(config);
	m_Volume=SpiFlash::Create(spi_host);
	Console::Print("OK\n");
	#else
	m_Volume=Storage::FlashMemory::Create("flash.bin");
	#endif
	Console::Print("Reading page 0...");
	auto page=Page::Create(m_Volume);
	UINT64 time=SystemTimer::Microseconds();
	m_Volume->Read(0, 0, page);
	UINT64 time_read=SystemTimer::Microseconds()-time;
	Console::Print("OK (%u µs)\n\n", time_read);
	PrintPage(page);
	}, "test");
task->Then(this, [this, task]()
	{
	auto status=task->GetStatus();
	if(StatusHelper::Failed(status))
		{
		Console::Print("\nFailed (%u)\n", (UINT)status);
		}
	else
		{
		Console::Print("\nSuccess\n");
		}
	});
}


//================
// Common Private
//================

VOID Application::PrintBuffer(BYTE const* buf, UINT size)
{
CHAR hex[3];
for(UINT pos=0; pos<size; pos++)
	{
	StringHelper::Print(hex, 3, "%x", buf[pos]);
	Console::Print(hex);
	}
}

VOID Application::PrintPage(Page* page)
{
auto buf=page->Begin();
UINT pos=0;
WORD page_spare=0;
WORD page_size=m_Volume->GetPageSize(&page_spare);
WORD page_total=page_size+page_spare;
UINT line_len=32;
UINT line_count=page_total/line_len;
UINT line_spare=page_size/line_len;
for(UINT line=0; line<line_count; line++)
	{
	if(line==line_spare)
		Console::Print("\n");
	PrintBuffer(&buf[pos], line_len);
	pos+=line_len;
	Console::Print("\n");
	}
}

}