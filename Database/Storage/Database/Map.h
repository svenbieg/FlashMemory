//=======
// Map.h
//=======

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

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
class Map: public Object
{
public:
	// Common
	VOID Clear();
	BOOL Set(_key_t const& Key, _value_t const& Value);

private:
	// Con-/Destructors
	Map(Database* Database, UINT Block);
	static inline Handle<Map> Create(Database* Database, UINT Block) { return Object::Create<Map>(Database, Block); }

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