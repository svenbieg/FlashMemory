//============
// Platform.h
//============

#pragma once


//=========
// Windows
//=========

#define _WINSOCKAPI_
#define SECURITY_WIN32

#include <Windows.h>

#undef VOID

#undef min
#undef max

#undef TEXT

#undef DrawText
#undef InterlockedDecrement
#undef InterlockedIncrement

#undef MessageBox
#ifdef _UNICODE
#define MessageBoxT MessageBoxW
#else
#define MessageBoxT MessageBoxA
#endif

#define WM_DISPATCH (WM_USER+1)

#define ICO_APP 1


//======================
// Forward-Declarations
//======================

#define __COPY_MAX 2
#define __LONG long
#define __LONG_LONG long long

#ifdef _X86_
#define __ADDR_T unsigned long
#define __ISENSE_T unsigned long
#else
#define __ADDR_T unsigned long long
#define __ISENSE_T unsigned long long
#endif
