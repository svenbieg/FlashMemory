//=========
// Block.h
//=========

#pragma once


//=======
// Using
//=======

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


//=======
// Block
//=======

class Block: public Object, public Streams::RandomAccessStream
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static Handle<Block> Create(Volume* Volume, UINT Id);

	// Common
	VOID Erase();
	inline WORD GetPageCount()const { return m_PageCount; }
	UINT GetPosition()const;
	inline UINT GetSize()const { return m_Size; }
	VOID Skip();

	// Input-Stream
	SIZE_T Available()override;
	SIZE_T Read(VOID* Buffer, SIZE_T Size)override;

	// Output-Stream
	VOID Flush()override;
	SIZE_T Write(VOID const* Buffer, SIZE_T Size)override;

private:
	// Con-/Destructors
	Block(Volume* Volume, UINT Id);

	// Common
	UINT m_Id;
	Handle<Page> m_Page;
	WORD m_PageCount;
	WORD m_PageId;
	WORD m_PageSize;
	UINT m_Size;
	Handle<Volume> m_Volume;
	SkipBits z_SkipBits;
};

}