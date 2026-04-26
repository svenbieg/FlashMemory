//==============
// SerialPort.h
//==============

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/RandomAccessStream.h"
#include "Exception.h"
#include "Handle.h"


//===========
// Namespace
//===========

namespace Devices {
	namespace Serial {


//=============
// Serial-Port
//=============

class SerialPort: public Object, public Storage::Streams::RandomAccessStream
{
public:
	// Con-/Destructors
	static inline Handle<SerialPort> Create() { return new SerialPort(); }

	// Input-Stream
	SIZE_T Available()override;
	SIZE_T Read(VOID* Buffer, SIZE_T Size)override;

	// Output-Stream
	VOID Flush()override;
	SIZE_T Write(VOID const* Buffer, SIZE_T Size)override;

private:
	// Con-/Destructors
	SerialPort();

	// Common
	HANDLE m_Input;
	HANDLE m_Output;
};

}}