//==============
// SkipBits.cpp
//==============

#include "SkipBits.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {
		namespace Updates {


//==================
// Con-/Destructors
//==================

Handle<SkipBits> SkipBits::Create(Block* block)
{
UINT block_size=block->GetSize();
UINT chunk_count=block_size/CHUNK_SIZE;
UINT size=chunk_count/8;
return Object::CreateEx<SkipBits>(size, sizeof(SIZE_T), block);
}

Handle<SkipBits> SkipBits::Create(Volume* volume)
{
UINT block_size=volume->GetBlockSize();
UINT chunk_count=block_size/CHUNK_SIZE;
UINT size=chunk_count/8;
return Object::CreateEx<SkipBits>(size, sizeof(SIZE_T));
}


//========
// Common
//========

VOID SkipBits::Clear()
{
Fill(-1);
m_Changed=false;
m_SkipCount=0;
}

VOID SkipBits::Skip(UINT skip)
{
UINT skip_count=skip/CHUNK_SIZE;
if(m_SkipCount==skip_count)
	return;
auto bits=Begin();
for(UINT pos=m_SkipCount/ITEM_BITS; pos<m_Count; pos++)
	{
	UINT skip_pos=pos*ITEM_BITS;
	if(skip_pos>=skip_count)
		break;
	UINT mask=-1;
	UINT shift=TypeHelper::Min(skip_count-skip_pos, ITEM_BITS);
	mask>>=shift;
	mask<<=shift;
	bits[pos]=mask;
	}
m_SkipCount=skip_count;
m_Changed=true;
}


//==========================
// Con-/Destructors Private
//==========================

SkipBits::SkipBits(BYTE* buf, SIZE_T size):
Array(buf, size),
m_Changed(false),
m_SkipCount(0)
{
Fill(-1);
}

SkipBits::SkipBits(BYTE* buf, SIZE_T size, Block* block):
Array(buf, size),
m_Changed(false),
m_SkipCount(0)
{
UINT id=0;
block->Read(&id, sizeof(UINT));
if(id!=SKIP_ID)
	throw InvalidArgumentException();
block->Read(buf, size);
auto bits=Begin();
for(UINT pos=0; pos<m_Count; pos++)
	{
	UINT skip=Cpu::CountTrailingZeros(bits[pos]);
	if(!skip)
		break;
	m_SkipCount+=skip;
	}
UINT block_pos=block->GetPosition();
UINT block_skip=m_SkipCount*CHUNK_SIZE;
if(block_skip>block_pos)
	block->Seek(block_skip);
}

}}}