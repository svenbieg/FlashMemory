//=============
// MT29Flash.h
//=============

#pragma once


//=======
// Using
//=======

#include "Devices/Spi/SpiHost.h"
#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Devices {
	namespace Flash {


//============
// MT29-Flash
//============

class MT29Flash: public Storage::Volume
{
public:
	// Using
	using Page=Storage::Page;
	using SpiHost=Devices::Spi::SpiHost;

	// Friends
	friend Object;

	// Con-/Destructors
	static inline Handle<MT29Flash> Create(SpiHost* Host) { return Object::Create<MT29Flash>(Host); }

	// Common
	inline WORD GetId()const { return m_Id; }

	// Volume
	VOID Erase(UINT Block)override;
	UINT GetBlockSize()override;
	WORD GetPageSize(WORD* Spare=nullptr)override;
	UINT64 GetSize()override;
	Handle<Page> ReadPage(UINT Block, WORD Page)override;
	VOID Write(UINT Block, WORD Page, WORD Position, VOID const* Buffer, WORD Size)override;

protected:
	// Con-/Destructors
	MT29Flash(SpiHost* Host);

	// Common
	BYTE GetFeature(BYTE Feature);
	WORD ReadId();
	VOID Reset();
	VOID SetFeature(BYTE Feature, BYTE Value);
	BYTE Wait(BYTE Mask, BYTE Value, UINT Timeout=5);
	VOID WriteDisable();
	VOID WriteEnable();
	UINT m_BlockSize;
	WORD m_Id;
	WORD m_PageCount;
	WORD m_PageSize;
	WORD m_PageSpare;
	WORD m_PageTotal;
	UINT m_Size;
	Handle<SpiHost> m_SpiHost;
};

}}