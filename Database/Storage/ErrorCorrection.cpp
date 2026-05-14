//=====================
// ErrorCorrection.cpp
//=====================

// Checksum of variable sized blocks.

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database/wiki/Storage#Error-Correction

#include "ErrorCorrection.h"


//=======
// Using
//=======

#include "Storage/Page.h"
#include "BitHelper.h"
#include "FlagHelper.h"


//===========
// Namespace
//===========

namespace Storage {


//==========
// Settings
//==========

const BYTE ECC_SQUARE[]		={ 2,  4,  6,  8,  10,  12,  14,  16 };
const WORD ECC_PAYLOAD[]	={ 4, 16, 36, 64, 100, 144, 196, 256 };
const WORD ECC_SIZE[]		={ 8, 24, 48, 80, 120, 168, 224, 288 };


//================
// Common Private
//================

WORD ErrorCorrection::Available(Page* page)
{
WORD page_pos=m_Next;
if(page_pos==0)
	page_pos=page->m_Position;
WORD page_size=page->m_Size;
if(page_size-page_pos<10)
	return 0;
auto buf=page->Begin();
WORD size_bits=0;
MemoryHelper::Copy(&size_bits, &buf[page_pos], sizeof(WORD));
if(size_bits==0||size_bits==0xFFFF)
	return 0;
UINT size=GetSize(page, size_bits);
page_pos+=sizeof(WORD);
Correct(&buf[page_pos], size);
page->m_Position=page_pos;
m_Next=page_pos+ECC_SIZE[size];
return ECC_PAYLOAD[size];
}

VOID ErrorCorrection::ChecksumX(BYTE* sum_x, WORD size, BYTE const* buf)
{
BYTE square=ECC_SQUARE[size];
WORD pos=0;
for(BYTE y=0; y<square; y++)
	{
	sum_x[y]=0;
	for(BYTE x=0; x<square; x++)
		{
		sum_x[y]+=buf[pos];
		pos++;
		}
	}
}

VOID ErrorCorrection::ChecksumY(BYTE* sum_y, WORD size, BYTE const* buf)
{
BYTE square=ECC_SQUARE[size];
for(BYTE x=0; x<square; x++)
	{
	sum_y[x]=0;
	WORD pos=x;
	for(BYTE y=0; y<square; y++)
		{
		sum_y[x]+=buf[pos];
		pos+=square;
		}
	}
}

VOID ErrorCorrection::Correct(BYTE* buf, WORD size)
{
BYTE err_x[16];
BYTE y=0;
BYTE errc_x=ErrorX(err_x, size, buf, &y);
if(errc_x==0)
	return;
BYTE err_y[16];
BYTE x=0;
BYTE errc_y=ErrorY(err_y, size, buf, &x);
if(errc_y==0)
	return;
if(errc_x==1)
	{
	CorrectX(err_y, y, size, buf);
	return;
	}
if(errc_y==1)
	{
	CorrectY(err_x, x, size, buf);
	return;
	}
if(errc_x==2)
	{
	BYTE square=ECC_SQUARE[size];
	BYTE square_2=square/2;
	MemoryHelper::Zero(err_x, square_2);
	MemoryHelper::Copy(&err_x[square_2], &err_y[square_2], square_2);
	CorrectX(err_x, y, size, buf);
	MemoryHelper::Copy(err_x, err_y, square_2);
	MemoryHelper::Zero(&err_x[square_2], square_2);
	CorrectX(err_x, y+1, size, buf);
	return;
	}
throw ChecksumException();
}

VOID ErrorCorrection::CorrectX(BYTE const* err_y, BYTE y, WORD size, BYTE* buf)
{
BYTE square=ECC_SQUARE[size];
WORD pos=y*square;
BYTE chk=0;
for(BYTE x=0; x<square; x++)
	{
	buf[pos]-=err_y[x];
	chk+=buf[pos];
	pos++;
	}
BYTE payload=ECC_PAYLOAD[size];
auto sum_x=&buf[payload];
if(sum_x[y]!=chk)
	throw ChecksumException();
}

VOID ErrorCorrection::CorrectY(BYTE const* err_x, BYTE x, WORD size, BYTE* buf)
{
BYTE square=ECC_SQUARE[size];
WORD pos=x;
BYTE chk=0;
for(BYTE y=0; y<square; y++)
	{
	buf[pos]-=err_x[y];
	chk+=buf[pos];
	pos+=square;
	}
BYTE payload=ECC_PAYLOAD[size];
auto sum_y=&buf[payload+square];
if(sum_y[x]!=chk)
	throw ChecksumException();
}

BYTE ErrorCorrection::ErrorX(BYTE* err_x, WORD size, BYTE const* buf, BYTE* y_ptr)
{
BYTE square=ECC_SQUARE[size];
WORD payload=ECC_PAYLOAD[size];
auto sum_x=&buf[payload];
BYTE count=0;
WORD pos=0;
for(BYTE y=0; y<square; y++)
	{
	err_x[y]=0;
	for(BYTE x=0; x<square; x++)
		{
		err_x[y]+=buf[pos];
		pos++;
		}
	err_x[y]-=sum_x[y];
	if(err_x[y])
		{
		if(!count)
			*y_ptr=y;
		count++;
		}
	}
return count;
}

BYTE ErrorCorrection::ErrorY(BYTE* err_y, WORD size, BYTE const* buf, BYTE* x_ptr)
{
BYTE square=ECC_SQUARE[size];
WORD payload=ECC_PAYLOAD[size];
auto sum_y=&buf[payload+square];
BYTE count=0;
for(BYTE x=0; x<square; x++)
	{
	err_y[x]=0;
	WORD pos=x;
	for(BYTE y=0; y<square; y++)
		{
		err_y[x]+=buf[pos];
		pos+=square;
		}
	err_y[x]-=sum_y[x];
	if(err_y[x])
		{
		if(!count)
			*x_ptr=x;
		count++;
		}
	}
return count;
}

WORD ErrorCorrection::GetSize(Page* page, WORD bits)
{
auto buf=(BYTE const*)&bits;
WORD size=0;
if(GetSize(buf[0], &size))
	return size;
if(GetSize(buf[1], &size))
	return size;
throw ChecksumException();
}

BOOL ErrorCorrection::GetSize(BYTE bits, WORD* size_ptr)
{
UINT lsb=BitHelper::CountTrailingZeros(bits);
UINT mask=1U<<lsb;
if(bits!=mask)
	return false;
*size_ptr=lsb;
return true;
}

}