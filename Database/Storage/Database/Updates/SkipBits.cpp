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

VOID SkipBits::Skip(Block* block)
{
auto buf=(UINT const*)block->BeginRead();
m_BlockBitsPosition=block->GetPosition();
m_BlockSkipCount=SkipCount(buf, m_BlockBitsCount);
block->Skip(m_BlockSkipCount*sizeof(UINT));
if(m_BlockSkipCount)
	{
	block->SetPage(m_BlockSkipCount);
	buf=(UINT const*)block->BeginRead();
	}
m_PageBitsPosition=block->GetPosition();
m_PageSkipCount=SkipCount(buf, m_PageBitsCount);
block->Skip(m_PageSkipCount*sizeof(UINT));
UINT page_pos=block->GetPagePosition();
UINT skip_pos=m_PageSkipCount*CHUNK_SIZE;
if(skip_pos>page_pos)
	block->SetPagePosition(skip_pos);
}

SIZE_T SkipBits::WriteBlockBits(Block* block, UINT skip)
{
SIZE_T size=m_BlockBitsCount*sizeof(UINT);
if(!block)
	return size;
m_BlockBitsPosition=block->GetPosition();
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
m_PageBitsPosition=block->GetPosition();
UINT skip=skip_bytes/CHUNK_SIZE;
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
m_BlockBitsCount(0),
m_BlockBitsPosition(0),
m_BlockSkipCount(0),
m_PageBitsCount(0),
m_PageBitsPosition(0),
m_PageSkipCount(0)
{
UINT block_size=volume->GetBlockSize();
UINT page_size=volume->GetPageSize();
UINT page_count=block_size/page_size;
UINT chunk_count=page_size/CHUNK_SIZE;
m_BlockBitsCount=page_count/32;
m_PageBitsCount=chunk_count/32;
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