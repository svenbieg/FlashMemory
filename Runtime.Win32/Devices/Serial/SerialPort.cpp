//================
// SerialPort.cpp
//================

#include "SerialPort.h"


//===========
// Namespace
//===========

namespace Devices {
	namespace Serial {


//==============
// Input-Stream
//==============

SIZE_T SerialPort::Available()
{
return 0;
}

SIZE_T SerialPort::Read(VOID* buf, SIZE_T size)
{
return 0;
}


//===============
// Output-Stream
//===============

VOID SerialPort::Flush()
{
}

SIZE_T SerialPort::Write(VOID const* buf, SIZE_T size)
{
return 0;
}


//==========================
// Con-/Destructors Private
//==========================

SerialPort::SerialPort():
m_Input(GetStdHandle(STD_INPUT_HANDLE)),
m_Output(GetStdHandle(STD_OUTPUT_HANDLE))
{

}

}}