//=============
// MapUpdate.h
//=============

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/InputStream.h"


//======================
// Forward-Declarations
//======================

namespace Storage
{
namespace Database
	{
	template <class _key_t, class _value_t, class _size_t, WORD _group_size> class Map;
	}
}


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {
		namespace Updates {


//============
// Map-Update
//============

template <class _key_t, class _value_t, class _size_t, WORD _group_size>
class MapUpdate
{
public:
	// Using
	using InputStream=Storage::Streams::InputStream;

	// Friends
	friend Map<_key_t, _value_t, _size_t, _group_size>;

private:
	// Update
	enum class Update: WORD
		{
		None,
		RootSet='RS'
		};

	// Common
	SIZE_T ReadFromStream(InputStream* Stream)
		{
		SIZE_T size=0;
		while(1)
			{
			WORD skip=0;
			size+=Stream->Read(&skip, sizeof(WORD));
			if(skip!=-1)
				{
				size+=Stream->Read(nullptr, skip);
				continue;
				}
			Update update(Update::None);
			size+=Stream->Read(&update, sizeof(Update));

			}
		
		return 0;
		}
};

}}}