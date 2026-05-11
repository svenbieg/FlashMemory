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
	WORD GetAlignment()override;
	UINT GetBlockSize()override;
	UINT GetPageSize()override;
	UINT64 GetSize()override;
	VOID Read(UINT64 Position, VOID* Buffer, SIZE_T Size)override;
	VOID SetSize(UINT64 Size)override;
	VOID Write(UINT64 Position, VOID const* Buffer, SIZE_T Size)override;

protected:
	// Con-/Destructors
	SpiFlash(SpiHost* Host);

	// Common
	VOID OnSpiHostDataReceived();
	WORD ReadId();
	VOID Reset();
	UINT m_BlockSize;
	WORD m_Id;
	WORD m_PageSize;
	WORD m_PageSpare;
	UINT m_Size;
	Handle<SpiHost> m_SpiHost;
};

}}