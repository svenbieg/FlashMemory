//===============
// Redirection.h
//===============

// Bad blocks are redirected on backup-failure.

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database/wiki/Storage#Redirection

#pragma once


//=======
// Using
//=======

#include "Collections/index.hpp"
#include "Storage/Streams/InputStream.h"
#include "Storage/Streams/OutputStream.h"
#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Storage {


//=============
// Redirection
//=============

class Redirection: public Object
{
public:
	// Settings
	static const UINT REDIR_SIZE=4;

	// Using
	using InputStream=Storage::Streams::InputStream;
	using OutputStream=Storage::Streams::OutputStream;

	// Friends
	friend Object;

	// Con-/Destructors
	static inline Handle<Redirection> Create(Volume* Volume)
		{
		return Object::Create<Redirection>(Volume);
		}
	static inline Handle<Redirection> Create(Volume* Volume, UINT Block)
		{
		return Object::Create<Redirection>(Volume, Block);
		}

private:
	// Con-/Destructors
	Redirection(Volume* Volume);
	Redirection(Volume* Volume, UINT Block);

	// Common
	WORD ReadFromStream(InputStream* Stream);
	Collections::index<UINT, BYTE, 8> m_Blocks;
	WORD m_Size;
	Volume* m_Volume;
};

}