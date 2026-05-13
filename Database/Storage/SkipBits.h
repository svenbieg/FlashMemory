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
	friend Page;

	// Common
	SIZE_T ReadFromPage(Page* Page);
	SIZE_T WriteToStream(OutputStream* Stream, WORD SkipCount);

private:
	// Con-/Destructors
	SkipBits(Block* Block);
	SkipBits(Page* Page);

	// Common
	static UINT GetBits(UINT Position, UINT Skip);
	WORD m_BitsCount;
	WORD m_SkipCount;
};

}