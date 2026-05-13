//==============
// SkipBits.cpp
//==============

#include "SkipBits.h"


//=======
// Using
//=======

#include "Storage/Block.h"
#include "BitHelper.h"


//===========
// Namespace
//===========

namespace Storage {


//========
// Common
//========

SIZE_T SkipBits::ReadFromPage(Page* page)
{
m_SkipCount=0;
WORD page_pos=page->m_Position;
auto buf=page->Begin();
WORD pos=page_pos;
for(UINT u=0; u<m_BitsCount; u++)
	{
	UINT bits=0;
	MemoryHelper::Copy(&bits, &buf[pos], sizeof(UINT));
	UINT skip=BitHelper::CountTrailingZeros(bits);
	if(!skip)
		break;
	m_SkipCount+=skip;
	pos+=sizeof(UINT);
	}
WORD size=m_SkipCount*sizeof(UINT);
page->m_Position=page_pos+size;
return size;
}

SIZE_T SkipBits::WriteToStream(OutputStream* stream, WORD skip_count)
{
SIZE_T size=m_BitsCount*sizeof(UINT);
if(!stream)
	return size;
for(UINT u=0; u<m_BitsCount; u++)
	{
	UINT bits=GetBits(u*32, skip_count);
	stream->Write(&bits, sizeof(UINT));
	}
return size;
}


//==========================
// Con-/Destructors Private
//==========================

SkipBits::SkipBits(Block* block):
m_BitsCount(block->GetPageCount()/32),
m_SkipCount(0)
{}

SkipBits::SkipBits(Page* page):
m_BitsCount(page->GetSize()/32),
m_SkipCount(0)
{}


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

}