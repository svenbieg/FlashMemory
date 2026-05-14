//=======
// Map.h
//=======

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Storage/Database/Updates/MapUpdate.h"
#include "Storage/Database/Entry.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//=====
// Map
//=====

template <class _key_t, class _value_t, class _size_t=UINT, WORD _group_size=10>
class Map: public Entry
{
public:
	// Using
	using _update_t=Storage::Database::Updates::MapUpdate<_key_t, _value_t, _size_t, _group_size>;

	// Friends
	friend Object;

	// Common
	VOID Clear();
	BOOL Set(_key_t const& Key, _value_t const& Value);

protected:
	// Common
	SIZE_T ReadFromStream(InputStream* Stream)override
		{
		SIZE_T size=0;
		size+=Entry::ReadFromStream(Stream);
		if(m_Id!=MAP_ID)
			throw NotFoundException();
		size+=_update_t::ReadFromStream(Stream, this);
		return size;
		}
	SIZE_T WriteToStream(OutputStream* Stream)override
		{
		throw NotImplementedException();
		return 0;
		}

private:
	// Settings
	static const UINT MAP_ID=ENTRY_ID('MAP ');

	// Con-/Destructors
	Map(Database* Database): Entry(Database, -1, MAP_ID) {}
	Map(Database* Database, UINT Block): Entry(Database, Block, MAP_ID)
		{
		auto volume=GetVolume();
		auto block=Block::Create(volume, m_Block);
		ReadFromStream(block);
		block->Skip();
		_update_t::ReadFromStream(block, this, &m_Update);
		m_Size=block->GetPosition();
		}
	static inline Handle<Map> Create(Database* Database, UINT Block=-1)
		{
		return Entry::Create<Map>(Database, Block);
		}

	// Group
	class Group: public Entry
		{
		public:
			// Common
			virtual _key_t const& GetFirstKey();
			virtual _key_t const& GetLastKey();
			virtual WORD GetLevel();
			virtual BOOL Set(_key_t const& Key, _value_t const& Value);
		};

	// Item-Group
	class ItemGroup: public Group
		{
		public:
			// Common
			_key_t const& GetFirstKey()override;
			_key_t const& GetLastKey()override;
			WORD GetLevel()override { return 0; }
			BOOL Set(_key_t const& Key, _value_t const& Value)override;
		};

	// Parent-Group
	class ParentGroup: public Group
		{
		public:
			// Common
			_key_t const& GetFirstKey()override;
			_key_t const& GetLastKey()override;
			WORD GetLevel()override;
			BOOL Set(_key_t const& Key, _value_t const& Value)override;
		};

	// Common
	Handle<Group> m_Root;
};

}}