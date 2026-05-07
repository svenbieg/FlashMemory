//============
// SkipBits.h
//============

#pragma once


//=======
// Using
//=======

#include "Collections/Array.h"
#include "Storage/Database/Block.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {
		namespace Updates {


//===========
// Skip-Bits
//===========

class SkipBits: public Collections::Array<UINT>
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static Handle<SkipBits> Create(Block* Block);
	static Handle<SkipBits> Create(Volume* Volume);

	// Common
	VOID Clear();
	VOID Skip(UINT Position);

private:
	// Settings
	static const UINT CHUNK_SIZE=128;
	static const UINT ITEM_BITS=sizeof(UINT)*8;
	static const UINT ITEM_SIZE=sizeof(UINT);
	static const UINT SKIP_ID='SKIP';

	// Con-/Destructors
	SkipBits(BYTE* Buffer, SIZE_T Size);
	SkipBits(BYTE* Buffer, SIZE_T Size, Block* Block);

	// Common
	BOOL m_Changed;
	UINT m_SkipCount;
};

}}}