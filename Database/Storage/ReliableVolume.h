//==================
// ReliableVolume.h
//==================

// Wear-leveling is redirecting bad blocks.

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database/wiki/Storage#Reliable-Volume

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
	virtual VOID Erase(UINT Block)override
		{
		UINT redir=Redirect(Block);
		try
			{
			_base_t::Erase(redir);
			}
		catch(ErrorException e)
			{
			redir=Spare(Block);
			_base_t::Erase(redir);
			}
		}
	virtual VOID ReadPage(UINT Block, WORD Id, Page* Page)override
		{
		Block=Redirect(Block);
		_base_t::ReadPage(Block, Id, Page);
		}
	virtual BOOL SetSize(UINT64 Size)override
		{
		return Size+m_Spare<=this->m_Size;
		}
	virtual VOID Write(UINT Block, WORD Page, WORD Position, VOID const* Buffer, WORD Size)override
		{
		UINT redir=Redirect(Block);
		try
			{
			_base_t::WritePage(redir, Page, Position, Buffer, Size);
			}
		catch(ErrorException)
			{
			UINT spare=Spare(Block);
			if(Page>0)
				{
				WORD page_size=this->m_PageSize;
				auto page=Page::Create(this);
				auto buf=page->Begin();
				for(UINT pos=0; pos<Page; pos++)
					{
					_base_t::ReadPage(redir, pos, page);
					_base_t::Write(spare, pos, 0, buf, page_size);
					}
				}
			if(Position>0)
				{
				auto page=Page::Create(this);
				auto buf=page->Begin();
				_base_t::ReadPage(redir, Page, page);
				_base_t::Write(spare, Page, 0, buf, Position);
				}
			redir=spare;
			_base_t::Write(redir, Page, Position, Buffer, Size);
			}
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
		auto page=Page::Create(this);
		_base_t::ReadPage(0, 0, page);
		auto entries=(UINT*)page->Begin();
		if(Create==FileCreateMode::CreateAlways)
			entries[0]=0;
		if(entries[0]!=REDIR_ID)
			{
			if(Create==FileCreateMode::OpenExisting)
				throw NotFoundException();
			entries[0]=REDIR_ID;
			_base_t::Erase(0);
			_base_t::Write(0, 0, 0, entries, sizeof(UINT));
			return;
			}
		if(Create==FileCreateMode::CreateNew)
			throw AlreadyExistsException();
		UINT count=(this->m_PageSize/sizeof(UINT))-2;
		for(UINT pos=1; pos<count; pos+=2)
			{
			if(entries[pos]==-1)
				{
				m_RedirectPosition=pos*sizeof(UINT);
				break;
				}
			if(entries[pos]==~entries[pos+1])
				{
				m_Redirect.set(entries[pos], m_RedirectCount++);
				m_RedirectPosition=(pos+2)*sizeof(UINT);
				}
			}
		}

private:
	// Settings
	static const UINT REDIR_ID='RIDR';
	static const UINT ENTRY_SIZE=2*sizeof(UINT);

	// Common
	UINT Redirect(UINT Block)
		{
		Block+=m_Spare;
		UINT redir=0;
		if(m_Redirect.try_get(Block, &redir))
			return redir;
		return Block;
		}
	UINT Spare(UINT Block)
		{
		if(m_RedirectCount==m_Spare)
			throw ErrorException();
		UINT redir=m_RedirectCount++;
		m_Redirect.set(Block, redir);
		UINT entry[2];
		entry[0]=Block;
		entry[1]=~Block;
		if(!_base_t::Write(m_RedirectPosition, entry, ENTRY_SIZE))
			return false;
		m_RedirectPosition+=ENTRY_SIZE;
		return redir;
		}
	Collections::map<UINT, UINT, BYTE, 8> m_Redirect;
	UINT m_RedirectCount;
	UINT m_RedirectPosition;
	UINT m_Spare;
};

}