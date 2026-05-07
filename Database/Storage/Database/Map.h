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
	static const UINT MAP_ID='MAP';

	// Con-/Destructors
	Map(Database* Database, UINT Block): Entry(Database, Block)
		{
		m_BlockPosition=m_Block->GetPosition();
		m_Block=nullptr;
		}
	Map(Database* Database, UINT Block, EntryCreateMode Create): Entry(Database, Block, Create)
		{}
	static inline Handle<Map> Create(Database* Database, UINT Block, EntryCreateMode Create)
		{
		return Entry::Create<Map>(Database, Block, Create);
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