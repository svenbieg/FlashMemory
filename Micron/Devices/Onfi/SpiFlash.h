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

	// Volume
	VOID Erase(UINT64 Offset, UINT Size);
	UINT GetBlockSize();
	UINT GetPageSize();
	UINT64 GetSize();
	VOID Read(UINT64 Position, VOID* Buffer, SIZE_T Size);
	VOID SetSize(UINT64 Size);
	VOID Write(UINT64 Position, VOID const* Buffer, SIZE_T Size);

protected:
	// Con-/Destructors
	SpiFlash(SpiHost* Host);

	// Common
	UINT m_BlockSize;
	UINT m_PageSize;
	UINT64 m_Size;
	Handle<SpiHost> m_SpiHost;
};

}}