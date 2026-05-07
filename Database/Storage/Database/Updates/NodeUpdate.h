//==============
// NodeUpdate.h
//==============

#pragma once


//=======
// Using
//=======

#include "Collections/Array.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"
#include "StringClass.h"


//======================
// Forward-Declarations
//======================

namespace Storage
{
namespace Database
	{
	class Node;
	}
}


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {
		namespace Updates {


//=============
// Node-Update
//=============

class NodeUpdate
{
public:
	// Con-/Destructors
	virtual ~NodeUpdate() {}

protected:
	// Using
	using StreamReader=Storage::Streams::StreamReader;
	using StreamWriter=Storage::Streams::StreamWriter;

	// Friends
	friend Node;

	// Con-/Destructors
	NodeUpdate(): m_Next(nullptr) {}
	template <class _update_t, class... _args_t> static VOID Create(NodeUpdate** Next, _args_t... Arguments)
		{
		while(*Next)
			Next=&(*Next)->m_Next;
		*Next=new _update_t(Arguments...);
		}

	// Common
	static SIZE_T ReadFromStream(StreamReader& Reader, Node* Node);
	virtual SIZE_T WriteToStream(StreamWriter& Writer)=0;
	static SIZE_T WriteToStream(StreamWriter& Writer, Node* Node);

	// Common
	NodeUpdate* m_Next;
};


//=========
// Updates
//=========

class NodeUpdateAttributeRemove: public NodeUpdate
{
private:
	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateAttributeRemove(UINT Position): m_Position(Position) {}

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override;
	UINT m_Position;
};

class NodeUpdateAttributeSet: public NodeUpdate
{
private:
	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateAttributeSet(UINT Position, Handle<String> Value):
		m_Position(Position),
		m_Value(Value)
		{}
	NodeUpdateAttributeSet(Handle<String> Key, Handle<String> Value):
		m_Key(Key),
		m_Position(0),
		m_Value(Value)
		{}

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override
		{
		return WriteToStream(Writer, m_Position, m_Key, m_Value);
		}
	static SIZE_T WriteToStream(StreamWriter& Writer, UINT Position, Handle<String> Key, Handle<String> Value);
	Handle<String> m_Key;
	UINT m_Position;
	Handle<String> m_Value;
};

class NodeUpdateChildAppend: public NodeUpdate
{
private:
	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateChildAppend(Node* Child): m_Child(Child) {}

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override
		{
		return WriteToStream(Writer, m_Child);
		}
	static SIZE_T WriteToStream(StreamWriter& Writer, Node* Child);
	Handle<Node> m_Child;
};

class NodeUpdateChildRemove: public NodeUpdate
{
private:
	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateChildRemove(UINT Position): m_Position(Position) {}

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override;
	UINT m_Position;
};

class NodeUpdateChildSelect: public NodeUpdate
{
private:
	// Using
	using PositionArray=Collections::Array<UINT>;

	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateChildSelect(PositionArray* Position): m_Position(Position) {}

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override;
	Handle<PositionArray> m_Position;
};

class NodeUpdateClear: public NodeUpdate
{
private:
	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateClear()=default;

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override;
};

class NodeUpdateTagSet: public NodeUpdate
{
private:
	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateTagSet(Handle<String> Tag): m_Tag(Tag) {}

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override
		{
		return WriteToStream(Writer, m_Tag);
		}
	static SIZE_T WriteToStream(StreamWriter& Writer, Handle<String> Tag);
	Handle<String> m_Tag;
};

class NodeUpdateValueSet: public NodeUpdate
{
private:
	// Friends
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateValueSet(Handle<String> Value): m_Value(Value) {}

	// Common
	SIZE_T WriteToStream(StreamWriter& Writer)override
		{
		return WriteToStream(Writer, m_Value);
		}
	static SIZE_T WriteToStream(StreamWriter& Writer, Handle<String> Value);
	Handle<String> m_Value;
};

}}}