//==================
// ReversedVolume.h
//==================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database/wiki/Storage#Reversed-Volume

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


//=================
// Reversed-Volume
//=================

template <class _base_t> class ReversedVolume: public _base_t
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	template <class... _args_t> static inline Handle<Volume> Create(_args_t... Arguments)
		{
		return Object::Create<ReversedVolume>(Arguments...);
		}

	// Volume
	virtual VOID Erase(UINT64 Offset, UINT Size)override
		{
		UINT64 reverse=this->m_Size-Offset-Size;
		_base_t::Erase(reverse, Size);
		}
	virtual VOID Read(UINT64 Offset, VOID* Buffer, SIZE_T Size)override
		{
		UINT64 reverse=Reverse(Offset, Size);
		_base_t::Read(reverse, Buffer, Size);
		}
	virtual VOID Write(UINT64 Offset, VOID const* Buffer, SIZE_T Size)override
		{
		UINT64 reverse=Reverse(Offset, Size);
		_base_t::Write(reverse, Buffer, Size);
		}

protected:
	// Con-/Destructors
	template <class... _args_t> ReversedVolume(_args_t... Arguments):
		_base_t(Arguments...),
		m_BlockCount(this->m_Size/this->m_BlockSize) {}

private:
	// Common
	UINT64 Reverse(UINT64 Offset, SIZE_T Size)
		{
		UINT block_size=this->m_BlockSize;
		UINT block=Offset/block_size;
		UINT block_pos=Offset%block_size;
		SIZE_T copy=TypeHelper::AlignUp(Size, block_size);
		UINT block_count=copy/block_size;
		block=m_BlockCount-block-block_count;
		return block*block_size+block_pos;
		}
	UINT m_BlockCount;
};

}