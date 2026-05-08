//========
// Onfi.h
//========

#pragma once


//=======
// Using
//=======

#include "TypeHelper.h"


//===========
// Namespace
//===========

namespace Devices {
	namespace Onfi {


//==========
// Commands
//==========

enum class OnfiCommand: BYTE
{
EraseBlock			=0xD8,
GetFeatures			=0x0F,
ProgramLoad			=0x02,
ProgramExecute		=0x10,
ProgramLoadRandom	=0x84,
ReadCache			=0x03,
ReadCacheLast		=0x3F,
ReadCacheRandom		=0x30,
ReadId				=0x9F,
ReadPage			=0x13,
Reset				=0xFF,
SetFeatures			=0x1F
};

}}