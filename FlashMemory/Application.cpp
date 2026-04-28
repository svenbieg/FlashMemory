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
#include "Storage/EmbeddedVolume.h"
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
Console::Print("Creating database...\n");
#ifdef _PICO2W
auto spi_host=SpiHost::Create();
auto volume=EmbeddedVolume<SpiFlash>::Create(FileCreateMode::OpenAlways, 64, spi_host);
#else
auto volume=Storage::FlashMemory::Create("flash.bin");
#endif
m_Database=Database::Create(volume, FileCreateMode::OpenAlways);
}

}