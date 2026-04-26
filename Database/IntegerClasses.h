//==================
// IntegerClasses.h
//==================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Storage/Encoding/Dwarf.h"


//==========
// Unsigned
//==========

class Unsigned
{
public:
	// Using
	using Dwarf=Storage::Encoding::Dwarf;
	using InputStream=Storage::Streams::InputStream;
	using OutputStream=Storage::Streams::OutputStream;

	// Con-/Destructors
	inline Unsigned(): m_Value(0) {}
	inline Unsigned(UINT64 Value): m_Value(Value) {}

	// Common
	inline operator UINT64()const { return m_Value; }
	inline SIZE_T ReadFromStream(InputStream* Stream) { return Dwarf::ReadUnsigned(Stream, &m_Value); }
	inline SIZE_T WriteToStream(OutputStream* Stream) { return Dwarf::WriteUnsigned(Stream, m_Value); }

private:
	// Common
	UINT64 m_Value;
};
