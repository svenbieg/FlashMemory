//=============
// XmlNode.cpp
//=============

#include "XmlNode.h"


//=======
// Using
//=======

#include "Concurrency/ReadLock.h"
#include "Concurrency/WriteLock.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"

using namespace Concurrency;
using namespace Storage::Streams;


//===========
// Namespace
//===========

namespace Storage {
	namespace Xml {


//========
// Common
//========

VOID XmlNode::AppendChild(XmlNode* child)
{
this->AppendChild(child, EventNotification::None);
Changed(this);
}

VOID XmlNode::AppendChild(XmlNode* child, EventNotification notify)
{
assert(child);
auto name=child->GetName();
if(name)
	{
	ReadLock lock(m_Mutex);
	if(m_Index.contains(name))
		throw AlreadyExistsException();
	}
WriteLock lock(m_Mutex);
m_Children.append(child);
if(name)
	m_Index.set(name, child);
}

VOID XmlNode::Clear()
{
if(Clear(EventNotification::None))
	Changed(this);
}

BOOL XmlNode::Clear(EventNotification notify)
{
WriteLock lock(m_Mutex);
bool cleared=false;
cleared|=m_Attributes.clear();
cleared|=m_Children.clear();
cleared|=m_Index.clear();
cleared|=m_Value;
if(!cleared)
	return false;
m_Value=nullptr;
return true;
}

VOID XmlNode::CopyFrom(XmlNode* node)
{
if(!node)
	throw InvalidArgumentException();
WriteLock lock(m_Mutex);
ReadLock node_lock(node->m_Mutex);
m_Tag=node->m_Tag;
m_Attributes.copy_from(node->m_Attributes);
m_Children.clear();
m_Index.clear();
if(node->m_Value)
	{
	m_Value=node->m_Value;
	}
else
	{
	for(auto it=node->m_Children.cbegin(); it.has_current(); it.move_next())
		{
		auto child=XmlNode::Create();
		child->CopyFrom(it.get_current());
		m_Children.append(child);
		child->m_Parent=this;
		auto name=child->GetName();
		if(name)
			m_Index.set(name, child);
		}
	}
}

Handle<String> XmlNode::GetAttribute(Handle<String> key)
{
ReadLock lock(m_Mutex);
return m_Attributes.get(key);
}

BOOL XmlNode::GetAttribute(Handle<String> key, UINT* value_ptr)
{
ReadLock lock(m_Mutex);
Handle<String> value;
if(!m_Attributes.try_get(key, &value))
	return false;
lock.Unlock();
return value->Scan("%u", value_ptr)==1;
}

BOOL XmlNode::GetAttribute(Handle<String> key, UINT64* value_ptr)
{
ReadLock lock(m_Mutex);
Handle<String> value;
if(!m_Attributes.try_get(key, &value))
	return false;
lock.Unlock();
return value->Scan("%u", value_ptr)==1;
}

Handle<XmlNodeAttributeIterator> XmlNode::GetAttributes()
{
return new XmlNodeAttributeIterator(this);
}

Handle<XmlNode> XmlNode::GetChild(Handle<String> name)
{
ReadLock lock(m_Mutex);
return m_Index.get(name);
}

Handle<XmlNode> XmlNode::GetChildAt(UINT pos)
{
ReadLock lock(m_Mutex);
return m_Children.get_at(pos);
}

Handle<XmlNodeChildIterator> XmlNode::GetChildren()
{
return new XmlNodeChildIterator(this);
}

Handle<String> XmlNode::GetName()
{
ReadLock lock(m_Mutex);
Handle<String> name;
if(!m_Attributes.try_get("Name", &name))
	return nullptr;
return name;
}

Handle<String> XmlNode::GetTag()
{
ReadLock lock(m_Mutex);
return m_Tag;
}

Handle<String> XmlNode::GetValue()
{
ReadLock lock(m_Mutex);
return m_Value;
}

BOOL XmlNode::HasAttribute(Handle<String> key)
{
ReadLock lock(m_Mutex);
return m_Attributes.contains(key);
}

VOID XmlNode::InsertChildAt(UINT pos, XmlNode* child)
{
if(InsertChildAt(pos, child, EventNotification::None))
	Changed(this);
}

BOOL XmlNode::InsertChildAt(UINT pos, XmlNode* child, EventNotification notify)
{
if(!child)
	return false;
auto name=child->GetName();
if(name)
	{
	ReadLock lock(m_Mutex);
	if(m_Index.contains(name))
		throw AlreadyExistsException();
	}
WriteLock lock(m_Mutex);
m_Children.insert_at(pos, child);
if(name)
	m_Index.set(name, child);
return true;
}

SIZE_T XmlNode::ReadFromStream(InputStream* stream)
{
WriteLock lock(m_Mutex);
m_Attributes.clear();
m_Children.clear();
m_Index.clear();
m_Parent=nullptr;
m_Value=nullptr;
StreamReader reader(stream);
SIZE_T read=0;
auto value=reader.ReadString(&read, "<", "\r\n\t ");
if(value)
	{
	if(m_Tag)
		throw InvalidArgumentException();
	m_Value=value;
	return read;
	}
if(!CharHelper::Equal(reader.LastChar, '<'))
	throw InvalidArgumentException();
auto tag=reader.ReadString(&read, " />", " ");
if(!tag)
	throw InvalidArgumentException();
if(m_Tag)
	{
	if(StringHelper::Compare(m_Tag, tag, 0, false)!=0)
		throw InvalidArgumentException();
	}
while(CharHelper::Equal(reader.LastChar, ' '))
	{
	auto att_name=reader.ReadString(&read, " =/>", " ");
	if(!att_name)
		break;
	Handle<String> att_value;
	if(CharHelper::Equal(reader.LastChar, '='))
		att_value=reader.ReadString(&read, "\"", " \"");
	m_Attributes.set(att_name, att_value);
	}
if(CharHelper::Equal(reader.LastChar, '/'))
	{
	read+=reader.ReadChar();
	if(!CharHelper::Equal(reader.LastChar, '>'))
		throw InvalidArgumentException();
	return read;
	}
while(1)
	{
	auto child=XmlNode::Create();
	read+=child->ReadFromStream(stream);
	if(!child->m_Tag)
		{
		m_Value=child->m_Value;
		break;
		}
	m_Children.append(child);
	child->m_Parent=this;
	auto name=child->GetName();
	if(!name)
		continue;
	if(!m_Index.add(name, child))
		return 0;
	}
read+=reader.ReadChar();
if(!CharHelper::Equal(reader.LastChar, '/'))
	throw InvalidArgumentException();
auto close=reader.ReadString(&read, ">");
if(!CharHelper::Equal(reader.LastChar, '>'))
	throw InvalidArgumentException();
if(StringHelper::Compare(close, tag, 0, false)!=0)
	throw InvalidArgumentException();
m_Tag=tag;
return read;
}

VOID XmlNode::RemoveAttribute(Handle<String> key)
{
if(RemoveAttribute(key, EventNotification::None))
	Changed(this);
}

BOOL XmlNode::RemoveAttribute(Handle<String> key, EventNotification notify)
{
WriteLock lock(m_Mutex);
return RemoveAttributeInternal(key);
}

VOID XmlNode::RemoveChildAt(UINT pos)
{
RemoveChildAt(pos, EventNotification::None);
Changed(this);
}

VOID XmlNode::RemoveChildAt(UINT pos, EventNotification notify)
{
WriteLock lock(m_Mutex);
m_Children.remove_at(pos);
}

VOID XmlNode::SetAttribute(Handle<String> key, Handle<String> value)
{
if(SetAttribute(key, value, EventNotification::None))
	Changed(this);
}

BOOL XmlNode::SetAttribute(Handle<String> key, Handle<String> value, EventNotification notify)
{
WriteLock lock(m_Mutex);
return SetAttributeInternal(key, value);
}

VOID XmlNode::SetName(Handle<String> name)
{
if(SetName(name, EventNotification::None))
	Changed(this);
}

BOOL XmlNode::SetName(Handle<String> name, EventNotification notify)
{
WriteLock lock(m_Mutex);
return SetNameInternal(name);
}

VOID XmlNode::SetTag(Handle<String> tag)
{
if(SetTag(tag, EventNotification::None))
	Changed(this);
}

BOOL XmlNode::SetTag(Handle<String> tag, EventNotification notify)
{
WriteLock lock(m_Mutex);
if(m_Tag==tag)
	return false;
m_Tag=tag;
return true;
}

VOID XmlNode::SetValue(Handle<String> value)
{
if(SetValue(value, EventNotification::None))
	Changed(this);
}

BOOL XmlNode::SetValue(Handle<String> value, EventNotification notify)
{
WriteLock lock(m_Mutex);
if(m_Value==value)
	return false;
m_Children.clear();
m_Index.clear();
m_Value=value;
return true;
}

SIZE_T XmlNode::WriteToStream(OutputStream* stream, INT level)
{
ReadLock lock(m_Mutex);
if(!m_Tag)
	return 0;
StreamWriter writer(stream);
SIZE_T written=0;
INT next_level=-1;
if(level>=0)
	{
	next_level=level+1;
	written+=writer.Print("\r\n");
	}
if(level>0)
	written+=writer.PrintChar('\t', level);
written+=writer.Print("<");
written+=writer.Print(m_Tag);
for(auto it=m_Attributes.cbegin(); it.has_current(); it.move_next())
	{
	auto name=it.get_key();
	auto value=it.get_value();
	written+=writer.Print(" ");
	written+=writer.Print(name);
	if(value)
		{
		written+=writer.Print("=\"");
		written+=writer.Print(value);
		written+=writer.Print("\"");
		}
	}
if(m_Value)
	{
	written+=writer.Print(">");
	written+=writer.Print(m_Value);
	written+=writer.Print("</");
	written+=writer.Print(m_Tag);
	written+=writer.Print(">");
	return written;
	}
if(!m_Children.get_count())
	{
	written+=writer.Print(" />");
	return written;
	}
written+=writer.Print(">");
for(auto it=m_Children.cbegin(); it.has_current(); it.move_next())
	{
	auto child=it.get_current();
	written+=child->WriteToStream(stream, next_level);
	}
if(level>=0)
	written+=writer.Print("\r\n");
if(level>0)
	written+=writer.PrintChar('\t', level);
written+=writer.Print("</");
written+=writer.Print(m_Tag);
written+=writer.Print(">");
return written;
}


//============================
// Con-/Destructors Protected
//============================

XmlNode::XmlNode(Handle<String> tag):
m_Parent(nullptr),
m_Tag(tag)
{}


//==================
// Common Protected
//==================

BOOL XmlNode::RemoveAttributeInternal(Handle<String> key)
{
if(StringHelper::Compare(key, "Name", 0, false)==0)
	return SetNameInternal(nullptr);
return m_Attributes.remove(key);
}

BOOL XmlNode::SetAttributeInternal(Handle<String> key, Handle<String> value)
{
if(StringHelper::Compare(key, "Name", 0, false)==0)
	return SetNameInternal(value);
return m_Attributes.set(key, value);
}

BOOL XmlNode::SetNameInternal(Handle<String> name)
{
Handle<String> old_name;
m_Attributes.try_get("Name", &old_name);
if(old_name==name)
	return false;
if(m_Parent)
	{
	WriteLock parent_lock(m_Parent->m_Mutex);
	if(name)
		{
		if(!m_Parent->m_Index.add(name, this))
			throw AlreadyExistsException();
		}
	if(old_name)
		m_Parent->m_Index.remove(old_name);
	}
if(name)
	{
	m_Attributes.set("Name", name);
	}
else
	{
	m_Attributes.remove("Name");
	}
return true;
}

}}