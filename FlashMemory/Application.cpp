//=================
// Application.cpp
//=================

#include "Application.h"


//=======
// Using
//=======

#include "Concurrency/Scheduler.h"
#include "UI/Console.h"

using namespace Concurrency;
using namespace FlashMemory;
using namespace Storage;
using namespace UI;

#ifdef _PICO2W

#include "Devices/Onfi/SpiFlash.h"
#include "Devices/Pio/SpiEmulator.h"
#include "Storage/ReliableVolume.h"

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
#ifdef _PICO2W
auto task=Task::Create(this, [this]()
	{
	Console::Print("Initializing flash-chip...");
	SpiConfiguration config;
	config.Divisor=2;
	config.PinChipSelect=GpioPin::Gpio17;
	config.PinClock=GpioPin::Gpio18;
	config.PinRx=GpioPin::Gpio16;
	config.PinTx=GpioPin::Gpio19;
	auto spi_host=SpiEmulator::Create(config);
	auto spi_flash=SpiFlash::Create(spi_host);
	Console::Print("OK\n");
	//auto volume=ReliableVolume<SpiFlash>::Create(FileCreateMode::OpenAlways, 64, spi_host);
	}, "test");
task->Then(this, [this]()
	{
	Console::Print("Done\n");
	});
#else
auto volume=Storage::FlashMemory::Create("flash.bin");
Console::Print("Creating database...\n");
m_Database=Database::Create(volume, FileCreateMode::OpenAlways);
auto editor=m_Database->Edit();
#endif
}

}