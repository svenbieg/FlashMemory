//=========
// Entry.h
//=========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Collections/Array.h"
#include "Storage/Streams/RandomAccessStream.h"
#include "Storage/File.h"
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

class Node;


//=======
// Entry
//=======

class Entry: public Object, public Storage::Streams::RandomAccessStream
{
public:
	// Friends
	friend Node;
	friend Object;

	// Common
	inline UINT GetPosition()const { return m_Position; }

	// Input-Stream
	SIZE_T Available()override;
	SIZE_T Read(VOID* Buffer, SIZE_T Size)override;

	// Output-Stream
	VOID Flush()override;
	SIZE_T Write(VOID const* Buffer, SIZE_T Size)override;

private:
	// Using
	using SkipBitArray=Collections::Array<UINT>;

	// Con-/Destructors
	Entry(BYTE* Buffer, SIZE_T Size, Volume* Volume, UINT Block, UINT Id, FileCreateMode Create);
	static Handle<Entry> Create(Volume* Volume, UINT Block, UINT Id, FileCreateMode Create=FileCreateMode::OpenExisting);

	// Common
	Handle<SkipBitArray> CreateSkipBits(UINT Skip=0);
	VOID SkipPages(SkipBitArray* SkipBits);
	UINT m_BlockSize;
	BYTE* m_Buffer;
	UINT m_Limit;
	UINT64 m_Offset;
	UINT m_Page;
	UINT m_PageSize;
	UINT m_Position;
	Handle<Volume> m_Volume;
	UINT m_Written;
};

}}