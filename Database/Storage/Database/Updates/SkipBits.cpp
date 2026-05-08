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

VOID SkipBits::Skip(Block* block, UINT* block_ptr, UINT* page_ptr)
{
UINT block_size=block->GetSize();
UINT page_size=block->GetPageSize();
UINT page_count=block_size/page_size;
UINT skip_block=page_count/32;
auto buf=(UINT const*)block->BeginRead();
*block_ptr=block->GetPosition();
skip_block=SkipCount(buf, skip_block);
if(skip_block)
	{
	block->SetPage(skip_block);
	buf=(UINT const*)block->BeginRead();
	}
UINT chunk_count=page_size/CHUNK_SIZE;
UINT skip_page=chunk_count/32;
*page_ptr=block->GetPosition();
skip_page=SkipCount(buf, skip_page);
UINT page_pos=block->GetPagePosition();
if(skip_page>page_pos)
	block->SetPagePosition(skip_page);
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