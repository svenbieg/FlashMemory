//==========
// Volume.h
//==========

#pragma once


//=======
// Using
//=======

#include "Storage/Block.h"


//===========
// Namespace
//===========

namespace Storage {


//========
// Volume
//========

class Volume: public Object
{
public:
	// Using
	using Block=Storage::Block;
	using Page=Storage::Page;

	// Common
	virtual VOID Erase(UINT64 Offset, UINT Size)=0;
	virtual UINT GetBlockSize()=0;
	virtual WORD GetPageSize(WORD* Spare=nullptr)=0;
	virtual UINT64 GetSize()=0;
	virtual VOID ReadPage(UINT Block, WORD Id, Page* Page)=0;
	virtual VOID SetSize(UINT64 Size)=0;
	virtual VOID Write(UINT64 Position, VOID const* Buffer, SIZE_T Size)=0;

protected:
	// Con-/Destructors
	Volume()=default;
};

}