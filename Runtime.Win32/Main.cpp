//==========
// Main.cpp
//==========

#include "TypeHelper.h"


//=======
// Using
//=======

#include "UI/Console.h"

using namespace UI;

extern VOID Main();


//=============
// Entry-Point
//=============

INT main(LPCSTR cmd)
{
SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
Main();
return 0;
}
