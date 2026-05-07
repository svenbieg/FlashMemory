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
	static VOID Skip(Block* Block);

private:
	// Common
	static UINT SkipCount(UINT const* SkipBits, UINT Size);
};

}}}