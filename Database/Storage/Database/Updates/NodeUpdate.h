//==============
// NodeUpdate.h
//==============

#pragma once


//=======
// Using
//=======

#include "Storage/Streams/InputStream.h"
#include "Storage/Streams/OutputStream.h"
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
	// Using
	using InputStream=Storage::Streams::InputStream;
	using OutputStream=Storage::Streams::OutputStream;

	// Friends
	friend Node;

	// Con-/Destructors
	virtual ~NodeUpdate()=default;

	// Common
	inline NodeUpdate** GetNext() { return &m_Next; }

protected:
	// Con-/Destructors
	NodeUpdate(Node* Node): m_Next(nullptr), m_Node(Node) {}

	// Common
	static NodeUpdate** GetUpdate(Node* Node);
	virtual SIZE_T WriteToStream(OutputStream* Stream)=0;
	NodeUpdate* m_Next;
	Node* m_Node;

private:
	// Common
	static SIZE_T ReadFromStream(InputStream* Stream, Node* Node);
	static SIZE_T WriteToStream(OutputStream* Stream, Node* Node);
};


//==================
// Attribute-Remove
//==================

class NodeUpdateAttributeRemove: public NodeUpdate
{
public:
	// Common
	inline Handle<String> GetKey()const { return m_Key; }

private:
	// Friends
	friend Node;
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateAttributeRemove(Node* Node, Handle<String> Key);
	static VOID Create(Node* Node, Handle<String> Key);

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	Handle<String> m_Key;
};


//===============
// Attribute-Set
//===============

class NodeUpdateAttributeSet: public NodeUpdate
{
public:
	// Common
	inline Handle<String> GetKey()const { return m_Key; }
	inline VOID SetValue(Handle<String> Value) { m_Value=Value; }

private:
	// Friends
	friend Node;
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateAttributeSet(Node* Node, Handle<String> Key, Handle<String> Value);
	static VOID Create(Node* Node, Handle<String> Key, Handle<String> Value);

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	static SIZE_T WriteToStream(OutputStream* Stream, Handle<String> Key, Handle<String> Value);
	Handle<String> m_Key;
	Handle<String> m_Value;
};


//==============
// Child-Append
//==============

class NodeUpdateChildAppend: public NodeUpdate
{
private:
	// Friends
	friend Node;
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateChildAppend(Node* Node, UINT Child);
	static VOID Create(Node* Node, UINT Child);

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	static SIZE_T WriteToStream(OutputStream* Stream, UINT Child);
	UINT m_Child;
};


//==============
// Child-Remove
//==============

class NodeUpdateChildRemove: public NodeUpdate
{
private:
	// Friends
	friend Node;
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateChildRemove(Node* Node, UINT Child);
	static VOID Create(Node* Node, UINT Child);

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	UINT m_Child;
};


//=======
// Clear
//=======

class NodeUpdateClear: public NodeUpdate
{
private:
	// Friends
	friend Node;
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateClear(Node* Node);
	static VOID Create(Node* Node);

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
};


//=========
// Tag-Set
//=========

class NodeUpdateTagSet: public NodeUpdate
{
private:
	// Friends
	friend Node;
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateTagSet(Node* Node, Handle<String> Tag);
	static VOID Create(Node* Node, Handle<String> Tag);

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	static SIZE_T WriteToStream(OutputStream* Stream, Handle<String> Tag);
	Handle<String> m_Tag;
};


//===========
// Value-Set
//===========

class NodeUpdateValueSet: public NodeUpdate
{
private:
	// Friends
	friend Node;
	friend NodeUpdate;

	// Con-/Destructors
	NodeUpdateValueSet(Node* Node, Handle<String> Value);
	static VOID Create(Node* Node, Handle<String> Value);

	// Common
	SIZE_T WriteToStream(OutputStream* Stream)override;
	static SIZE_T WriteToStream(OutputStream* Stream, Handle<String> Value);
	Handle<String> m_Value;
};

}}}