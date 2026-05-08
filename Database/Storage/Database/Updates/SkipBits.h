//============
// SkipBits.h
//============

#pragma once


//=======
// Using
//=======

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

class SkipBits
{
public:
	// Con-/Destructors
	static VOID Initialize(Block* Block);
	static VOID Skip(Block* Block, UINT* SkipBlock, UINT* SkipPage);

private:
	// Settings
	static constexpr UINT CHUNK_SIZE=32;

	// Common
	static UINT SkipCount(UINT const* SkipBits, UINT Size);
};

}}}