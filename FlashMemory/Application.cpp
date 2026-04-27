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

#ifndef _WIN32
#include "Devices/Onfi/SpiFlash.h"
#include "Storage/WearLeveling.h"
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
#ifndef _WIN32
auto spi_host=SpiHost::Create();
auto spi_flash=SpiFlash::Create(spi_host);
auto volume=WearLeveling::Create(spi_flash, FileCreateMode::OpenAlways);
#else
auto volume=Storage::FlashMemory::Create("flash.bin");
#endif
m_Database=Database::Create(volume, FileCreateMode::OpenAlways);
}

}