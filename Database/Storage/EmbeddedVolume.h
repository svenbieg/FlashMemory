//==================
// EmbeddedVolume.h
//==================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database/wiki/Storage#Embedded-Volume

// The embedded volume is a combination of reversed and reliable.

#pragma once


//=======
// Using
//=======

#include "Storage/ReliableVolume.h"
#include "Storage/ReversedVolume.h"


//===========
// Namespace
//===========

namespace Storage {


//=================
// Embedded-Volume
//=================

template <class _base_t> class EmbeddedVolume: public ReliableVolume<ReversedVolume<_base_t>>
{
public:
	// Friends
	friend Object;

	// Using
	using _reliable_t=ReliableVolume<ReversedVolume<_base_t>>;

	// Con-/Destructors
	template <class... _args_t> static inline Handle<Volume> Create(FileCreateMode Create, UINT Spare, _args_t... Arguments)
		{
		return Object::Create<EmbeddedVolume>(Create, Spare, Arguments...);
		}

private:
	// Con-/Destructors
	template <class... _args_t> EmbeddedVolume(FileCreateMode Create, UINT Spare, _args_t... Arguments):
		_reliable_t(Create, Spare, Arguments...)
		{}
};

}