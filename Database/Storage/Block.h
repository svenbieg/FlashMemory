//=========
// Block.h
//=========

#pragma once


//=======
// Using
//=======

#include "Storage/ErrorCorrection.h"
#include "Storage/Page.h"
#include "Storage/SkipBits.h"


//===========
// Namespace
//===========

namespace Storage {


//======================
// Forward-Declarations
//======================

class Volume;


//============
// Block-Mode
//============

enum class BlockMode: UINT
{
None=0,
ErrorCorrection=1,
SkipBits=2,
Entry=ErrorCorrection|SkipBits
};


//=======
// Block
//=======

class Block: public Object, public Streams::RandomAccessStream
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static Handle<Block> Create(Volume* Volume, UINT Id, BlockMode Mode=BlockMode::None);

	// Common
	VOID Erase();
	inline WORD GetPageCount()const { return m_PageCount; }
	inline UINT GetPosition()const { return m_Position; }
	inline UINT GetSize()const { return m_Size; }
	VOID Skip();

	// Input-Stream
	SIZE_T Available()override;
	SIZE_T Read(VOID* Buffer, SIZE_T Size)override;

	// Output-Stream
	VOID Flush()override;
	SIZE_T Write(VOID const* Buffer, SIZE_T Size)override;

private:
	// Flags
	enum class BlockFlags: UINT
		{
		None=0,
		ErrorCorrection=1,
		SkipBits=2,
		SkipPage=4
		};

	// Con-/Destructors
	Block(Volume* Volume, UINT Id, BlockMode Mode);

	// Common
	WORD m_Available;
	ErrorCorrection m_ErrorCorrection;
	BlockFlags m_Flags;
	UINT m_Id;
	Handle<Page> m_Page;
	WORD m_PageCount;
	WORD m_PageId;
	WORD m_PageSize;
	UINT m_Position;
	UINT m_Size;
	SkipBits m_SkipBlock;
	SkipBits m_SkipPage;
	Handle<Volume> m_Volume;
	WORD m_Writable;
};

}