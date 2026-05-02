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

	// Con-/Destructors
	virtual ~NodeOperation() {}

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


//============
// Operations
//============

class NodeOperationAttributeRemove: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationAttributeRemove(UINT Position): m_Position(Position) {}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	UINT m_Position;
};

class NodeOperationAttributeSet: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationAttributeSet(UINT Position, Handle<String> Value):
		m_Position(Position),
		m_Value(Value)
		{}
	NodeOperationAttributeSet(Handle<String> Key, Handle<String> Value):
		m_Key(Key),
		m_Position(0),
		m_Value(Value)
		{}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Key;
	UINT m_Position;
	Handle<String> m_Value;
};

class NodeOperationAttributeSetInteger: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationAttributeSetInteger(UINT Position, INT64 Value):
		m_Position(Position),
		m_Value(Value)
		{}
	NodeOperationAttributeSetInteger(Handle<String> Key, INT64 Value):
		m_Key(Key),
		m_Position(0),
		m_Value(Value)
		{}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Key;
	UINT m_Position;
	INT64 m_Value;
};

class NodeOperationChildAppend: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationChildAppend(Node* Child): m_Child(Child) {}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<Node> m_Child;
};

class NodeOperationChildInsert: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationChildInsert(UINT Position, Node* Child): m_Child(Child), m_Position(Position) {}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<Node> m_Child;
	UINT m_Position;
};

class NodeOperationChildRemove: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationChildRemove(UINT Position): m_Position(Position) {}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	UINT m_Position;
};

class NodeOperationChildSelect: public NodeOperation
{
private:
	// Using
	using PositionArray=Collections::Array<UINT>;

	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationChildSelect(PositionArray* Position): m_Position(Position) {}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<PositionArray> m_Position;
};

class NodeOperationClear: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationClear()=default;

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
};

class NodeOperationTagSet: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationTagSet(Handle<String> Tag): m_Tag(Tag) {}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Tag;
};

class NodeOperationValueSet: public NodeOperation
{
private:
	// Friends
	friend Node;

	// Con-/Destructors
	NodeOperationValueSet(Handle<String> Value): m_Value(Value) {}

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Value;
};

}}