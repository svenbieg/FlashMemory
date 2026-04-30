//===========
// XmlNode.h
//===========

#pragma once


//=======
// Using
//=======

#include "Collections/list.hpp"
#include "Collections/map.hpp"
#include "Concurrency/Scheduler.h"
#include "Storage/Streams/Stream.h"
#include "Event.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Xml {


//======================
// Forward-Declarations
//======================

class XmlNodeAttributeIterator;
class XmlNodeChildIterator;


//==========
// XML-Node
//==========

class XmlNode: public Object
{
public:
	// Using
	using InputStream=Storage::Streams::InputStream;
	using OutputStream=Storage::Streams::OutputStream;

	// Friends
	friend XmlNodeAttributeIterator;
	friend XmlNodeChildIterator;

	// Con-/Destructors
	static inline Handle<XmlNode> Create(Handle<String> Tag=nullptr) { return new XmlNode(Tag); }

	// Common
	VOID AppendChild(XmlNode* Child);
	VOID AppendChild(XmlNode* Child, EventNotification Notify);
	Event<XmlNode> Changed;
	VOID Clear();
	BOOL Clear(EventNotification Notify);
	VOID CopyFrom(XmlNode* Copy);
	Handle<String> GetAttribute(Handle<String> Name);
	BOOL GetAttribute(Handle<String> Name, UINT* Value);
	BOOL GetAttribute(Handle<String> Name, UINT64* Value);
	Handle<XmlNodeAttributeIterator> GetAttributes();
	Handle<XmlNode> GetChild(Handle<String> Name);
	Handle<XmlNode> GetChildAt(UINT Position);
	Handle<XmlNodeChildIterator> GetChildren();
	Handle<String> GetName();
	Handle<String> GetTag();
	Handle<String> GetValue();
	BOOL HasAttribute(Handle<String> Name);
	VOID InsertChildAt(UINT Position, XmlNode* Child);
	BOOL InsertChildAt(UINT Position, XmlNode* Child, EventNotification Notify);
	SIZE_T ReadFromStream(InputStream* Stream);
	VOID RemoveAttributeAt(UINT Position);
	VOID RemoveAttributeAt(UINT Position, EventNotification Notify);
	VOID RemoveChildAt(UINT Position);
	VOID RemoveChildAt(UINT Position, EventNotification Notify);
	inline VOID SetAttribute(Handle<String> Name, INT Value)
		{
		SetAttribute(Name, String::Create("%i", Value));
		}
	inline BOOL SetAttribute(Handle<String> Name, INT Value, EventNotification Notify)
		{
		return SetAttribute(Name, String::Create("%i", Value), Notify);
		}
	inline VOID SetAttribute(Handle<String> Name, UINT Value)
		{
		SetAttribute(Name, String::Create("%u", Value));
		}
	inline BOOL SetAttribute(Handle<String> Name, UINT Value, EventNotification Notify)
		{
		return SetAttribute(Name, String::Create("%u", Value), Notify);
		}
	inline VOID SetAttribute(Handle<String> Name, UINT64 Value)
		{
		SetAttribute(Name, String::Create("%u", Value));
		}
	inline BOOL SetAttribute(Handle<String> Name, UINT64 Value, EventNotification Notify)
		{
		return SetAttribute(Name, String::Create("%u", Value), Notify);
		}
	VOID SetAttribute(Handle<String> Name, Handle<String> Value);
	BOOL SetAttribute(Handle<String> Name, Handle<String> Value, EventNotification Notify);
	VOID SetAttributeAt(UINT Position, Handle<String> Value);
	BOOL SetAttributeAt(UINT Position, Handle<String> Value, EventNotification Notify);
	VOID SetName(Handle<String> Name);
	BOOL SetName(Handle<String> Name, EventNotification Notify);
	VOID SetTag(Handle<String> Tag);
	BOOL SetTag(Handle<String> Tag, EventNotification Notify);
	VOID SetValue(Handle<String> Value);
	BOOL SetValue(Handle<String> Value, EventNotification Notify);
	SIZE_T WriteToStream(OutputStream* Stream, INT Level=-1);

protected:
	// Con-/Destructors
	XmlNode(Handle<String> Tag=nullptr);

	// Common
	BOOL SetNameInternal(Handle<String> Name);
	Collections::map<Handle<String>, Handle<String>> m_Attributes;
	Collections::list<Handle<XmlNode>> m_Children;
	Collections::map<Handle<String>, XmlNode*> m_Index;
	Concurrency::Mutex m_Mutex;
	XmlNode* m_Parent;
	Handle<String> m_Tag;
	Handle<String> m_Value;
};


//====================
// Attribute-Iterator
//====================

class XmlNodeAttributeIterator: public Object
{
public:
	// Using
	using AccessMode=Concurrency::AccessMode;

	// Friends
	friend XmlNode;

	// Access
	inline Handle<String> GetName()const { return m_It.get_key(); }
	inline Handle<String> GetValue()const { return m_It.get_value(); }
	inline BOOL HasCurrent()const { return m_It.has_current(); }

	// Navigation
	inline BOOL Begin() { return m_It.begin(); }
	inline BOOL End() { return m_It.rbegin(); }
	inline UINT GetPosition() { return m_It.get_position(); }
	inline BOOL MoveNext() { return m_It.move_next(); }
	inline BOOL MovePrevious() { return m_It.move_previous(); }

private:
	// Con-/Destructors
	XmlNodeAttributeIterator(XmlNode* Node): m_It(&Node->m_Attributes), m_Node(Node)
		{
		m_Node->m_Mutex.Lock(AccessMode::ReadOnly);
		}
	~XmlNodeAttributeIterator()
		{
		m_Node->m_Mutex.Unlock(AccessMode::ReadOnly);
		}

	// Common
	typename Collections::map<Handle<String>, Handle<String>>::iterator m_It;
	Handle<XmlNode> m_Node;
};


//================
// Child-Iterator
//================

class XmlNodeChildIterator: public Object
{
public:
	// Using
	using AccessMode=Concurrency::AccessMode;

	// Friends
	friend XmlNode;

	// Access
	inline Handle<XmlNode> GetCurrent()const { return m_It.get_current(); }
	inline BOOL HasCurrent()const { return m_It.has_current(); }

	// Navigation
	inline BOOL Begin() { return m_It.begin(); }
	inline BOOL End() { return m_It.rbegin(); }
	inline UINT GetPosition() { return m_It.get_position(); }
	inline BOOL MoveNext() { return m_It.move_next(); }
	inline BOOL MovePrevious() { return m_It.move_previous(); }

private:
	// Con-/Destructors
	XmlNodeChildIterator(XmlNode* Node): m_It(&Node->m_Children), m_Node(Node)
		{
		m_Node->m_Mutex.Lock(AccessMode::ReadOnly);
		}
	~XmlNodeChildIterator()
		{
		m_Node->m_Mutex.Unlock(AccessMode::ReadOnly);
		}

	// Common
	typename Collections::list<Handle<XmlNode>>::iterator m_It;
	Handle<XmlNode> m_Node;
};

}}