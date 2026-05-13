//========
// Page.h
//========

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/RandomAccessStream.h"
#include "Storage/ErrorCorrection.h"
#include "Storage/SkipBits.h"
#include "Handle.h"


//===========
// Namespace
//===========

namespace Storage {


//======================
// Forward-Declarations
//======================

class Block;
class Volume;


//======
// Page
//======

class Page: public Object, public Streams::RandomAccessStream
{
public:
	// Friends
	friend Block;
	friend ErrorCorrection;
	friend Object;
	friend SkipBits;

	// Con-/Destructors
	static Handle<Page> Create(Volume* Volume);

	// Common
	inline BYTE* Begin()const { return m_Buffer; }
	WORD GetPosition()const;
	inline WORD GetSize()const { return m_Size; }

	// Input-Stream
	SIZE_T Available()override;
	SIZE_T Read(VOID* Buffer, SIZE_T Size)override;

	// Output-Stream
	VOID Flush()override;
	SIZE_T Write(VOID const* Buffer, SIZE_T Size)override;

private:
	// Flags
	enum class PageFlags: UINT
		{
		None=0,
		Last=(1<<0)
		};

	// Con-/Destructors
	Page(BYTE* Buffer, SIZE_T Size, Volume* Volume);

	// Common
	WORD m_Available;
	BYTE* m_Buffer;
	ErrorCorrection m_ErrorCorrection;
	PageFlags m_Flags;
	WORD m_Position;
	WORD m_Size;
	SkipBits z_SkipBits;
};

}