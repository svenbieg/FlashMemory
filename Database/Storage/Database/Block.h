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
	namespace Database {


//======================
// Forward-Declarations
//======================

class Database;


//=======
// Block
//=======

class Block: public Object, public Storage::Streams::RandomAccessStream
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static Handle<Block> Create(Database* Database, UINT Block);

	// Common
	BYTE const* BeginRead();
	UINT GetPagePosition()const;
	inline UINT GetPageSize()const { return m_PageSize; }
	inline UINT GetPosition()const { return m_Position; }
	inline UINT GetSize()const { return m_Size; }
	VOID SetPage(UINT Page);
	VOID SetPagePosition(UINT Position);
	VOID SetPosition(UINT Position);
	UINT Skip(UINT Size);

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