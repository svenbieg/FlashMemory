//============
// SpiFlash.h
//============

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
	namespace Onfi {


//===========
// SPI-Flash
//===========

class SpiFlash: public Storage::Volume
{
public:
	// Using
	using SpiHost=Devices::Spi::SpiHost;

	// Friends
	friend Object;

	// Con-/Destructors
	static inline Handle<SpiFlash> Create(SpiHost* Host) { return Object::Create<SpiFlash>(Host); }

	// Common
	inline WORD GetId()const { return m_Id; }

	// Volume
	VOID Erase(UINT64 Offset, UINT Size)override;
	UINT GetBlockSize()override;
	WORD GetPageSize(WORD* Spare=nullptr)override;
	UINT64 GetSize()override;
	VOID ReadPage(UINT Block, WORD Id, Page* Page)override;
	VOID SetSize(UINT64 Size)override;
	VOID Write(UINT64 Position, VOID const* Buffer, SIZE_T Size)override;

protected:
	// Con-/Destructors
	SpiFlash(SpiHost* Host);

	// Common
	BYTE GetFeatures(BYTE Feature);
	WORD ReadId();
	VOID Reset();
	VOID SetFeatures(BYTE Feature, BYTE Value);
	UINT m_BlockSize;
	WORD m_Id;
	WORD m_PageSize;
	WORD m_PageSpare;
	UINT m_Size;
	Handle<SpiHost> m_SpiHost;
};

}}