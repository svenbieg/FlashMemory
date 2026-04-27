//================
// WearLeveling.h
//================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database/wiki#wear-leveling

#pragma once


//=======
// Using
//=======

#include "Collections/map.hpp"
#include "Storage/File.h"
#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Storage {


//===============
// Wear-Leveling
//===============

class WearLeveling: public Volume
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static inline Handle<WearLeveling> Create(Volume* Volume, FileCreateMode Create, UINT Spare=64) { return Object::Create<WearLeveling>(Volume, Create, Spare); }

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
	WearLeveling(Volume* Volume, FileCreateMode Create, UINT Spare);

	// Common
	UINT64 Redirect(UINT64 Offset);
	UINT64 Spare(UINT64 Offset);
	UINT m_BlockSize;
	UINT m_Count;
	UINT m_PageSize;
	UINT m_Position;
	Collections::map<UINT, UINT, BYTE, 8> m_Redirect;
	UINT64 m_Size;
	UINT m_Spare;
	Handle<Volume> m_Volume;
};

}