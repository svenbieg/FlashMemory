//============
// SkipBits.h
//============

#pragma once


//=======
// Using
//=======

#include "Storage/Database/Block.h"


//======================
// Forward-Declarations
//======================

namespace Storage
{
namespace Database
	{
	class Entry;
	}
}


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {
		namespace Updates {


//===========
// Skip-Bits
//===========

class SkipBits
{
public:
	// Friends
	friend Entry;

	// Con-/Destructors
	VOID Skip(Block* Block);
	SIZE_T WriteBlockBits(Block* Block, UINT SkipPages);
	SIZE_T WritePageBits(Block* Block, UINT SkipBytes);

private:
	// Settings
	static const UINT CHUNK_SIZE=32;

	// Con-/Destructors
	SkipBits(Volume* Volume);

	// Common
	static UINT GetBits(UINT Position, UINT Skip);
	static UINT SkipCount(UINT const* SkipBits, UINT Count);
	UINT m_BlockBitsCount;
	UINT m_BlockBitsPosition;
	UINT m_BlockSkipCount;
	UINT m_PageBitsCount;
	UINT m_PageBitsPosition;
	UINT m_PageSkipCount;
};

}}}