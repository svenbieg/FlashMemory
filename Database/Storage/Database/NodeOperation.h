//=================
// NodeOperation.h
//=================

#pragma once


//=======
// Using
//=======

#include "Collections/Array.h"
#include "Storage/Streams/InputStream.h"
#include "Storage/Streams/OutputStream.h"
#include "StringClass.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//======================
// Forward-Declarations
//======================

class Node;


//================
// Node-Operation
//================

class NodeOperation
{
public:
	// Friends
	friend Node;

protected:
	// Using
	using InputStream=Storage::Streams::InputStream;
	using OutputStream=Storage::Streams::OutputStream;

	// Con-/Destructors
	NodeOperation(): m_Next(nullptr) {}

	// Common
	static SIZE_T ReadFromStream(Node* Target, InputStream* Stream);
	virtual SIZE_T WriteToStream(OutputStream* Stream)=0;

	// Common
	NodeOperation* m_Next;
};


//==================
// Attribute-Remove
//==================

class NodeOperationAttributeRemove: public NodeOperation
{
private:
	// Friends
	friend NodeOperation;

	// Con-/Destructors
	NodeOperationAttributeRemove(Handle<String> Key): m_Key(Key) {}

	// Common
	static SIZE_T ReadFromStream(Node* Target, InputStream* Stream);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Key;
};


//===============
// Attribute-Set
//===============

class NodeOperationAttributeSet: public NodeOperation
{
private:
	// Friends
	friend NodeOperation;

	// Con-/Destructors
	NodeOperationAttributeSet(Handle<String> Key, Handle<String> Value):
		m_Key(Key),
		m_Value(Value) {}

	// Common
	static SIZE_T ReadFromStream(Node* Target, InputStream* Stream);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Key;
	Handle<String> m_Value;
};


//==============
// Child-Append
//==============

class NodeOperationChildAppend: public NodeOperation
{
private:
	// Friends
	friend NodeOperation;

	// Con-/Destructors
	NodeOperationChildAppend(Node* Child): m_Child(Child) {}

	// Common
	static SIZE_T ReadFromStream(Node* Target, InputStream* Stream);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<Node> m_Child;
};


//==============
// Child-Insert
//==============

class NodeOperationChildInsert: public NodeOperation
{
private:
	// Friends
	friend NodeOperation;

	// Con-/Destructors
	NodeOperationChildInsert(UINT Position, Node* Child): m_Child(Child), m_Position(Position) {}

	// Common
	static SIZE_T ReadFromStream(Node* Target, InputStream* Stream);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<Node> m_Child;
	UINT m_Position;
};


//==============
// Child-Remove
//==============

class NodeOperationChildRemove: public NodeOperation
{
private:
	// Con-/Destructors
	NodeOperationChildRemove(UINT Position): m_Position(Position) {}

	// Friends
	friend NodeOperation;

	// Common
	static SIZE_T ReadFromStream(Node* Target, InputStream* Stream);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	UINT m_Position;
};


//==============
// Child-Select
//==============

class NodeOperationChildSelect: public NodeOperation
{
private:
	// Using
	using PositionArray=Collections::Array<UINT>;

	// Friends
	friend NodeOperation;

	// Con-/Destructors
	NodeOperationChildSelect(PositionArray* Position): m_Position(Position) {}

	// Common
	static SIZE_T ReadFromStream(Node* Target, InputStream* Stream, Node** Selected);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<PositionArray> m_Position;
};


//=======
// Clear
//=======

class NodeOperationClear: public NodeOperation
{
private:
	// Con-/Destructors
	NodeOperationClear()=default;

	// Friends
	friend NodeOperation;

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
};


//=========
// Tag-Set
//=========

class NodeOperationTagSet: public NodeOperation
{
private:
	// Con-/Destructors
	NodeOperationTagSet(Handle<String> Tag): m_Tag(Tag) {}

	// Friends
	friend NodeOperation;

	// Common
	static SIZE_T ReadFromStream(Node* Node, InputStream* Stream);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Tag;
};


//===========
// Value-Set
//===========

class NodeOperationValueSet: public NodeOperation
{
private:
	// Con-/Destructors
	NodeOperationValueSet(Handle<String> Value): m_Value(Value) {}

	// Friends
	friend NodeOperation;

	// Common
	static SIZE_T ReadFromStream(Node* Node, InputStream* Stream);
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Value;
};

}}