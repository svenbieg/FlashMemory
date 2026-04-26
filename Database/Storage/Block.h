//=========
// Block.h
//=========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/RandomAccessStream.h"
#include "Storage/Volume.h"
#include "Handle.h"


//===========
// Namespace
//===========

namespace Storage {


//=======
// Block
//=======

class Block: public Object, public Storage::Streams::RandomAccessStream
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static Handle<Block> Create(Volume* Volume, UINT Block=0, UINT Position=0);

	// Common
	VOID Seek(UINT Position);
	VOID Seek(UINT Block, UINT Position);

	// Input-Stream
	SIZE_T Available()override;
	SIZE_T Read(VOID* Buffer, SIZE_T Size)override;

	// Output-Stream
	VOID Flush()override;
	SIZE_T Write(VOID const* Buffer, SIZE_T Size)override;

private:
	// Con-/Destructors
	Block(BYTE* Buffer, SIZE_T Size, Volume* Volume, UINT Block, UINT Position);

	// Common
	BYTE* m_Buffer;
	UINT64 m_Offset;
	UINT m_Page;
	UINT m_PageSize;
	UINT m_Position;
	UINT m_Size;
	Handle<Volume> m_Volume;
	UINT m_Written;
};

}