//===================
// ErrorCorrection.h
//===================

#pragma once


//=======
// Using
//=======

#include "TypeHelper.h"


//===========
// Namespace
//===========

namespace Storage {


//======================
// Forward-Declarations
//======================

class Page;


//==================
// Error-Correction
//==================

class ErrorCorrection
{
public:
	// Friends
	friend Page;

private:
	// Con-/Destructors
	ErrorCorrection()=default;

	// Common
	WORD Available(Page* Page);
	VOID ChecksumX(BYTE* Checksum, WORD Size, BYTE const* Buffer);
	VOID ChecksumY(BYTE* Checksum, WORD Size, BYTE const* Buffer);
	VOID Correct(BYTE* Buffer, WORD Size);
	VOID CorrectX(BYTE const* ErrorY, BYTE Y, WORD Size, BYTE* Buffer);
	VOID CorrectY(BYTE const* ErrorX, BYTE X, WORD Size, BYTE* Buffer);
	BYTE ErrorX(BYTE* Error, WORD Size, BYTE const* Buffer, BYTE* Y);
	BYTE ErrorY(BYTE* Error, WORD Size, BYTE const* Buffer, BYTE* X);
	WORD GetSize(Page*, WORD Bits);
	BOOL GetSize(BYTE Bits, WORD* Size);
	WORD m_Next=0;
};

}