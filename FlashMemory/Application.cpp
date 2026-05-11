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
#include "Storage/ReliableVolume.h"
using namespace Devices::Onfi;
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
	Console::Print("Initializing flash-chip...\n");
	auto spi_host=SpiHost::Create();
	auto spi_flash=SpiFlash::Create(spi_host);
	Console::Print("OK\n");
	//auto volume=ReliableVolume<SpiFlash>::Create(FileCreateMode::OpenAlways, 64, spi_host);
	}, "test");
task->Then(this, [this]()
	{
	Console::Print("Failed\n");
	});
#else
auto volume=Storage::FlashMemory::Create("flash.bin");
Console::Print("Creating database...\n");
m_Database=Database::Create(volume, FileCreateMode::OpenAlways);
auto editor=m_Database->Edit();
#endif
}

}