//=================
// Application.cpp
//=================

#include "Application.h"


//=======
// Using
//=======

#include "Collections/map.hpp"
#include "Concurrency/Scheduler.h"
#include "Devices/Timers/SystemTimer.h"
#include "UI/Console.h"
#include "StatusHelper.h"

using namespace Collections;
using namespace Concurrency;
using namespace Devices::Timers;
using namespace FlashMemory;
using namespace Storage;
using namespace Storage::Streams;
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
auto task=Task::Create(app, [app](){ app->Run(); });
task->Then(nullptr, [task]()
	{
	auto status=task->GetStatus();
	if(StatusHelper::Failed(status))
		{
		Console::Print("Failed (%u)\n", (UINT)status);
		}
	else
		{
		Console::Print("Done\n");
		}
	});
DispatchedQueue::Enter();
}


//===========
// Namespace
//===========

namespace FlashMemory {


//========
// Common
//========

VOID Application::Run()
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
auto spi_flash=SpiFlash::Create(spi_host);
m_Volume=spi_flash;
Console::Print("OK\n");
#else
m_Volume=Storage::FlashMemory::Create("flash.bin");
#endif
auto page=ReadPage(0, 0);
TaskInfo();
m_StatusLed->Blink(500);
}


//==========================
// Con-/Destructors Private
//==========================

Application::Application()
{
m_StatusLed=StatusLed::Create();
m_SystemTimer=SystemTimer::Create();
m_TaskMonitor=TaskMonitor::Create();
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
Console::Print("\n");
}

Handle<Page> Application::ReadPage(UINT block, WORD page_id)
{
UINT block_size=m_Volume->GetBlockSize();
WORD page_size=m_Volume->GetPageSize();
WORD page_count=block_size/page_size;
Console::Print("Reading page %u...", block*page_count+page_id);
auto page=Page::Create(m_Volume);
UINT64 time=m_SystemTimer->Microseconds();
m_Volume->Read(block, page_id, page);
UINT64 time_read=m_SystemTimer->Microseconds()-time;
Console::Print("OK (%u µs)\n\n", time_read);
return page;
}

VOID Application::TaskInfo()
{
TASK_INFO info[10];
UINT count=m_TaskMonitor->GetTaskInfo(info, 10);
map<Handle<String>, TASK_INFO*> info_map;
UINT64 total_time=0;
for(UINT u=0; u<count; u++)
	{
	info_map.add(info[u].Name, &info[u]);
	total_time+=info[u].TotalTime;
	}
for(auto const& it: info_map)
	{
	auto name=it.get_key();
	auto info=it.get_value();
	SIZE_T stack_size=info->StackSize;
	SIZE_T stack_used=info->StackUsed;
	UINT64 time=info->TotalTime;
	UINT percent=time*100/total_time;
	Console::Print("%3u%% %-12s %4u/%-4u bytes\n", percent, name, stack_used, stack_size);
	}
Console::Print("\n");
}

}