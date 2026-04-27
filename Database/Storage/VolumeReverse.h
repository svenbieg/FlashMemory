//=================
// VolumeReverse.h
//=================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

// User-data is stored backwards on embedded OTA devices.

#pragma once


//=======
// Using
//=======

#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Storage {


//================
// Volume-Reverse
//================

class VolumeReverse: public Volume
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static inline Handle<VolumeReverse> Create(Volume* Volume) { return Object::Create<VolumeReverse>(Volume); }

	// Volume
	VOID Erase(UINT64 Offset, UINT Size);
	UINT GetBlockSize();
	UINT GetPageSize();
	UINT64 GetSize();
	VOID Read(UINT64 Position, VOID* Buffer, SIZE_T Size);
	VOID SetSize(UINT64 Size);
	VOID Write(UINT64 Position, VOID const* Buffer, SIZE_T Size);

private:
	// Con-/Destructors
	VolumeReverse(Volume* Volume);

	// Common
	UINT64 Redirect(UINT64 Position, SIZE_T Size);
	UINT m_BlockCount;
	UINT m_BlockSize;
	UINT m_PageSize;
	UINT64 m_Size;
	Handle<Volume> m_Volume;
};

}