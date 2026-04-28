//==================
// ReliableVolume.h
//==================

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

// Wear-leveling is redirecting bad blocks.

#pragma once


//=======
// Using
//=======

#include "Collections/map.hpp"
#include "Storage/Buffer.h"
#include "Storage/File.h"
#include "Storage/Volume.h"


//===========
// Namespace
//===========

namespace Storage {


//=================
// Reliable-Volume
//=================

template <class _base_t> class ReliableVolume: public _base_t
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	template <class... _args_t> static inline Handle<Volume> Create(FileCreateMode Create, UINT Spare, _args_t... Arguments)
		{
		return Object::Create<ReliableVolume>(Create, Spare, Arguments...);
		}

	// Volume
	virtual VOID Erase(UINT64 Offset, UINT Size)override
		{
		UINT64 redir=Redirect(Offset);
		try
			{
			_base_t::Erase(redir, Size);
			return;
			}
		catch(ErrorException e)
			{
			redir=Spare(Offset);
			}
		_base_t::Erase(redir, Size);
		}
	virtual VOID Read(UINT64 Offset, VOID* Buffer, SIZE_T Size)override
		{
		Offset=Redirect(Offset);
		_base_t::Read(Offset, Buffer, Size);
		}
	virtual VOID Write(UINT64 Offset, VOID const* Buffer, SIZE_T Size)override
		{
		UINT64 redir=Redirect(Offset);
		try
			{
			_base_t::Write(redir, Buffer, Size);
			return;
			}
		catch(ErrorException e)
			{
			UINT64 spare=Spare(Offset);
			UINT block_pos=redir%this->m_BlockSize;
			if(block_pos>0)
				{
				UINT src=redir/this->m_BlockSize;
				UINT dst=spare/this->m_BlockSize;
				auto buf=Buffer::Create(this->m_PageSize);
				auto buf_ptr=buf->Begin();
				for(UINT pos=0; pos<block_pos; )
					{
					UINT copy=TypeHelper::Min(block_pos-pos, this->m_PageSize);
					_base_t::Read(redir+pos, buf_ptr, copy);
					_base_t::Write(spare+pos, buf_ptr, copy);
					pos+=copy;
					}
				}
			redir=spare;
			}
		_base_t::Write(redir, Buffer, Size);
		}

protected:
	// Con-/Destructors
	template <class... _args_t> ReliableVolume(FileCreateMode Create, UINT Spare, _args_t... Arguments):
		_base_t(Arguments...),
		m_RedirectCount(1),
		m_RedirectPosition(0),
		m_Spare(Spare)
		{
		this->m_Size-=m_Spare*this->m_BlockSize;
		auto buf=Buffer::Create(this->m_PageSize);
		auto entries=(UINT*)buf->Begin();
		_base_t::Read(0, entries, this->m_PageSize);
		if(Create==FileCreateMode::CreateAlways)
			entries[0]=0;
		if(entries[0]!=REDIR_ID)
			{
			if(Create==FileCreateMode::OpenExisting)
				throw NotFoundException();
			entries[0]=REDIR_ID;
			_base_t::Erase(0, this->m_BlockSize);
			_base_t::Write(0, entries, sizeof(UINT));
			return;
			}
		if(Create==FileCreateMode::CreateNew)
			throw AlreadyExistsException();
		UINT count=(this->m_PageSize/sizeof(UINT))-2;
		for(UINT pos=1; pos<count; pos+=2)
			{
			if(entries[pos]==-1)
				return;
			if(entries[pos+1]==REDIR_ID)
				{
				m_Redirect.set(entries[pos], m_RedirectCount++);
				m_RedirectPosition=(pos+2)*sizeof(UINT);
				continue;
				}
			if(entries[pos+1]==0)
				{
				m_RedirectPosition=(pos+2)*sizeof(UINT);
				continue;
				}
			if(entries[pos+2]!=-1)
				throw ErrorException();
			entries[pos]=0;
			entries[pos+1]=0;
			_base_t::Write(pos*sizeof(UINT), &entries[pos], 2*sizeof(UINT));
			m_RedirectPosition=(pos+2)*sizeof(UINT);
			return;
			}
		throw ErrorException();
		}

private:
	// Settings
	static const UINT REDIR_ID='RDIR';

	// Common
	UINT64 Redirect(UINT64 Offset)
		{
		Offset+=m_Spare*this->m_BlockSize;
		UINT block=Offset/this->m_BlockSize;
		UINT redir=0;
		if(m_Redirect.try_get(block, &redir))
			{
			UINT block_pos=Offset%this->m_BlockSize;
			Offset=(UINT64)redir*this->m_BlockSize+block_pos;
			}
		return Offset;
		}
	UINT64 Spare(UINT64 Offset)
		{
		if(m_RedirectCount==m_Spare)
			throw AbortException();
		Offset+=m_Spare*this->m_BlockSize;
		UINT block=Offset/this->m_BlockSize;
		UINT redir=m_RedirectCount++;
		m_Redirect.set(block, redir);
		UINT entry[2];
		entry[0]=block;
		entry[1]=REDIR_ID;
		_base_t::Write(m_RedirectPosition, entry, 2*sizeof(UINT));
		m_RedirectPosition+=2*sizeof(UINT);
		UINT block_pos=Offset%this->m_BlockSize;
		return (UINT64)redir*this->m_BlockSize+block_pos;
		}
	Collections::map<UINT, UINT, BYTE, 8> m_Redirect;
	UINT m_RedirectCount;
	UINT m_RedirectPosition;
	UINT m_Spare;
};

}