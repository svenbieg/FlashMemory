//=========
// Cpu.cpp
//=========

#include "Cpu.h"


//=======
// Using
//=======

#include <chrono>

using namespace std::chrono;


//===========
// Namespace
//===========

namespace Devices {
	namespace System {


//========
// Common
//========

VOID Cpu::Delay(UINT us)
{
auto start=high_resolution_clock::now();
while(1)
	{
	auto elapsed=high_resolution_clock::now()-start;
	if(duration_cast<microseconds>(elapsed).count()>=us)
		break;
	}
}

}}