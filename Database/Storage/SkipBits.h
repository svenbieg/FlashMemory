//============
// SkipBits.h
//============

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/OutputStream.h"


//===========
// Namespace
//===========

namespace Storage {


//======================
// Forward-Declarations
//======================

class Block;
class Page;


//===========
// Skip-Bits
//===========

class SkipBits
{
public:
	// Using
	using OutputStream=Storage::Streams::OutputStream;

	// Friends
	friend Block;

protected:
	// Con-/Destructors
	SkipBits(WORD BitsCount);

	// Common
	inline WORD GetSize()const { return m_BitsCount*sizeof(UINT); }
	WORD ReadFromPage(Page* Page);
	WORD WriteToStream(OutputStream* Stream);
	WORD m_BitsCount;
	WORD m_SkipCount;

private:
	// Common
	static UINT GetBits(WORD Position, WORD Skip);
};

}