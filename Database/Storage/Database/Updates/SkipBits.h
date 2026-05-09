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
	inline WORD GetAlignment()const { return m_Alignment; }
	inline UINT GetBlockBitsSize()const { return m_BlockBitsCount*sizeof(UINT); }
	inline UINT GetPageBitsSize()const { return m_PageBitsCount*sizeof(UINT); }
	SIZE_T ReadBlockBits(Block* Block);
	SIZE_T ReadPageBits(Block* Block);
	VOID Skip(Block* Block);
	SIZE_T WriteBlockBits(Block* Block, UINT SkipPages);
	SIZE_T WritePageBits(Block* Block, UINT SkipBytes);

private:
	// Con-/Destructors
	SkipBits(Volume* Volume);

	// Common
	static UINT GetBits(UINT Position, UINT Skip);
	static UINT SkipCount(UINT const* SkipBits, UINT Count);
	WORD m_Alignment;
	WORD m_BlockBitsCount;
	WORD m_BlockSkipCount;
	WORD m_PageBitsCount;
	WORD m_PageSkipCount;
};

}}}