//==============
// SkipBits.cpp
//==============

#include "SkipBits.h"


//=======
// Using
//=======

#include "Devices/System/Cpu.h"
#include "Storage/Database/Database.h"

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

SIZE_T SkipBits::ReadBlockBits(Block* block)
{
auto buf=(UINT const*)block->BeginRead();
m_BlockSkipCount=SkipCount(buf, m_BlockBitsCount);
return block->Skip(m_BlockSkipCount*sizeof(UINT));
}

SIZE_T SkipBits::ReadPageBits(Block* block)
{
auto buf=(UINT const*)block->BeginRead();
m_PageSkipCount=SkipCount(buf, m_PageBitsCount);
return block->Skip(m_PageSkipCount*sizeof(UINT));
}

VOID SkipBits::Skip(Block* block)
{
UINT page_size=block->GetPageSize();
UINT pos=m_BlockSkipCount*page_size;
pos+=m_PageSkipCount*m_Alignment;
block->SetPosition(pos);
}

SIZE_T SkipBits::WriteBlockBits(Block* block, UINT skip)
{
SIZE_T size=m_BlockBitsCount*sizeof(UINT);
if(!block)
	return size;
for(UINT u=0; u<m_BlockBitsCount; u++)
	{
	UINT bits=GetBits(u*32, skip);
	block->Write(&bits, sizeof(UINT));
	}
return size;
}

SIZE_T SkipBits::WritePageBits(Block* block, UINT skip_bytes)
{
SIZE_T size=m_PageBitsCount*sizeof(UINT);
if(!block)
	return size;
UINT skip=skip_bytes/m_Alignment;
for(UINT u=0; u<m_PageBitsCount; u++)
	{
	UINT bits=GetBits(u*32, skip);
	block->Write(&bits, sizeof(UINT));
	}
return size;
}


//==========================
// Con-/Destructors Private
//==========================

SkipBits::SkipBits(Volume* volume):
m_Alignment(0),
m_BlockBitsCount(0),
m_BlockSkipCount(0),
m_PageBitsCount(0),
m_PageSkipCount(0)
{
WORD align=volume->GetAlignment();
m_Alignment=TypeHelper::Min(align, 4U);
UINT block_size=volume->GetBlockSize();
UINT page_size=volume->GetPageSize();
UINT page_count=block_size/page_size;
UINT chunk_count=page_size/m_Alignment;
m_BlockBitsCount=(WORD)(page_count/32);
m_PageBitsCount=(WORD)(chunk_count/32);
}


//================
// Common Private
//================

UINT SkipBits::GetBits(UINT pos, UINT skip)
{
if(pos>=skip)
	return -1;
UINT bits=-1;
UINT shift=TypeHelper::Min(skip-pos, 32);
bits>>=shift;
bits<<=shift;
return bits;
}

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