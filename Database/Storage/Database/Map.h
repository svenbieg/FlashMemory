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

private:
	// Settings
	static constexpr UINT MAP_ID=ENTRY_ID('MAP ');

	// Con-/Destructors
	Map(Database* Database, UINT Block): Entry(Database, Block)
		{
		if(m_BlockId==-1)
			return;
		auto block=Block::Create(m_Database, m_BlockId);
		UINT id=0;
		block->Read(&id, sizeof(UINT));
		if(id!=MAP_ID)
			throw InvalidArgumentException();
		SkipBits::Skip(block);
		m_BlockPosition=block->GetPosition();
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