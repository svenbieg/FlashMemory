//=========
// Block.h
//=========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Collections/Array.h"
#include "Storage/Streams/RandomAccessStream.h"
#include "Storage/Volume.h"
#include "Handle.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//=======
// Block
//=======

class Block: public Object, public Storage::Streams::RandomAccessStream
{
public:
	// Using
	using SkipBitArray=Collections::Array<UINT>;

	// Friends
	friend Object;

	// Con-/Destructors
	static Handle<Block> Create(Volume* Volume, UINT Block);

	// Common
	inline UINT GetPageSize()const { return m_PageSize; }
	inline UINT GetPosition()const { return m_Position; }
	inline UINT GetSize()const { return m_Size; }
	Handle<SkipBitArray> ReadSkipBits();
	VOID Seek(UINT Position);
	VOID Seek(UINT Block, UINT Position);
	VOID SkipPages(SkipBitArray* SkipBits);

	// Input-Stream
	SIZE_T Available()override;
	SIZE_T Read(VOID* Buffer, SIZE_T Size)override;

	// Output-Stream
	VOID Flush()override;
	SIZE_T Write(VOID const* Buffer, SIZE_T Size)override;

private:
	// Con-/Destructors
	Block(BYTE* Buffer, SIZE_T Size, Volume* Volume, UINT Block);

	// Common
	Handle<SkipBitArray> CreateSkipBits(UINT Skip=0);
	BYTE* m_Buffer;
	UINT64 m_Offset;
	UINT m_Page;
	UINT m_PageSize;
	UINT m_Position;
	UINT m_Size;
	Handle<Volume> m_Volume;
	UINT m_Written;
};

}}