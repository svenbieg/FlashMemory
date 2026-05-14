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
	virtual VOID Erase(UINT Block)=0;
	virtual UINT GetBlockSize()=0;
	virtual WORD GetPageSize(WORD* Spare=nullptr)=0;
	virtual UINT64 GetSize()=0;
	virtual VOID ReadPage(UINT Block, WORD Id, Page* Page)=0;
	virtual BOOL SetSize(UINT64 Size)=0;
	virtual VOID Write(UINT Block, WORD Page, WORD Position, VOID const* Buffer, WORD Size)=0;

protected:
	// Con-/Destructors
	Volume()=default;
};

}