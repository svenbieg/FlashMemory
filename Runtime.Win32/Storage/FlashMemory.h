//===============
// FlashMemory.h
//===============

#pragma once


//=======
// Using
//=======

#include "Storage/Volume.h"
#include "StringClass.h"


//===========
// Namespace
//===========

namespace Storage {


//==============
// Flash-Memory
//==============

class FlashMemory: public Volume
{
public:
	// Con-/Destructors
	static inline Handle<FlashMemory> Create(Handle<String> Path) { return new FlashMemory(Path); }

	// Volume
	VOID Erase(UINT64 Position, UINT Size);
	WORD GetAlignment()override;
	UINT GetBlockSize()override;
	UINT GetPageSize()override;
	UINT64 GetSize()override;
	VOID Read(UINT64 Position, VOID* Buffer, SIZE_T Size)override;
	VOID SetSize(UINT64 Size)override;
	VOID Write(UINT64 Position, VOID const* Buffer, SIZE_T Size)override;

private:
	// Settings
	static const UINT BLOCK_SIZE=4096;
	static const UINT PAGE_SIZE=512;

	// Con-/Destructors
	FlashMemory(Handle<String> Path);

	// Common
	UINT m_BlockSize;
	BYTE m_Erase[BLOCK_SIZE];
	HANDLE m_File;
};

}