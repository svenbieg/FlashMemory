//==============
// SkipBits.cpp
//==============

#include "SkipBits.h"


//=======
// Using
//=======

#include "Devices/System/Cpu.h"

using namespace Devices::System;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {
		namespace Updates {


//========
// Common
//========

VOID SkipBits::Initialize(Block* block)
{
UINT block_size=block->GetSize();
UINT page_size=block->GetPageSize();
UINT page_count=block_size/page_size;
UINT count=page_count/32;
UINT bits=-1;
for(UINT u=0; u<count; u++)
	block->Write(&bits, sizeof(UINT));
}

VOID SkipBits::Skip(Block* block)
{
UINT block_size=block->GetSize();
UINT page_size=block->GetPageSize();
UINT page_count=block_size/page_size;
UINT count=page_count/32;
auto buf=(UINT const*)block->BeginRead();
UINT skip_count=SkipCount(buf, count);
UINT pos=skip_count*page_size;
block->Seek(pos);
buf=(UINT const*)block->BeginRead();
skip_count=SkipCount(buf, 1);
UINT chunk_size=page_size/32;
pos+=skip_count*chunk_size;
block->Seek(pos);
}


//================
// Common Private
//================

UINT SkipBits::SkipCount(UINT const* bits, UINT count)
{
UINT skip_count=0;
for(UINT u=0; u<count; u++)
	{
	UINT skip=Cpu::CountTrailingZeros(bits[u]);
	if(skip==0)
		break;
	skip_count+=skip;
	}
return skip_count;
}

}}}