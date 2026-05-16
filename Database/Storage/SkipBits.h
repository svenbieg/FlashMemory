//============
// SkipBits.h
//============

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/InputStream.h"
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
	using InputStream=Storage::Streams::InputStream;
	using OutputStream=Storage::Streams::OutputStream;

	// Friends
	friend Block;

private:
	// Con-/Destructors
	SkipBits(WORD BitsCount);

	// Common
	static UINT GetBits(WORD Position, WORD Skip);
	inline WORD GetSize()const { return m_BitsCount*sizeof(UINT); }
	WORD ReadFromPage(Page* Page);
	WORD WriteToStream(OutputStream* Stream);
	WORD m_BitsCount;
	WORD m_SkipCount;
};

}