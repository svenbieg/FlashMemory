//================
// NodeUpdate.cpp
//================

#include "NodeUpdate.h"


//=======
// Using
//=======

#include "Storage/Database/Node.h"
#include "Storage/Encoding/Dwarf.h"
#include "FlagHelper.h"

using namespace Storage::Encoding;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {
		namespace Updates {


//========
// Update
//========

enum class Update: BYTE
{
None=0,
AttributeRemove,
AttributeSet,
ChildAppend,
ChildRemove,
Clear,
NodeBegin,
NodeEnd,
TagSet,
ValueSet
};


//=============
// Node-Update
//=============

NodeUpdate::NodeUpdate(Node* node):
EntryUpdate(node)
{}

SIZE_T NodeUpdate::ReadFromStream(InputStream* stream, Node* node, EntryUpdate** update_ptr)
{
SIZE_T size=0;
Update update=Update::None;
size+=stream->Read(&update, sizeof(Update));
if(update!=Update::NodeBegin)
	throw InvalidArgumentException();
while(stream->Available())
	{
	size+=stream->Read(&update, sizeof(Update));
	if(update==Update::None)
		continue;
	if(update==Update::NodeEnd)
		break;
	switch(update)
		{
		case Update::AttributeRemove:
			{
			Handle<String> key;
			size+=key.ReadFromStream(stream);
			node->m_Attributes.remove(key);
			if(update_ptr)
				{
				auto attr_remove=new NodeUpdateAttributeRemove(node, key);
				*update_ptr=attr_remove;
				update_ptr=&attr_remove->m_Next;
				}
			break;
			}
		case Update::AttributeSet:
			{
			Handle<String> key;
			size+=key.ReadFromStream(stream);
			Handle<String> value;
			size+=value.ReadFromStream(stream);
			node->m_Attributes.set(key, value);
			if(update_ptr)
				{
				auto attr_set=new NodeUpdateAttributeSet(node, key, value);
				*update_ptr=attr_set;
				update_ptr=&attr_set->m_Next;
				}
			return size;
			}
		case Update::ChildAppend:
			{
			UINT child=0;
			size+=Dwarf::ReadUnsigned(stream, &child);
			node->m_Children.append(child);
			if(update_ptr)
				{
				auto child_append=new NodeUpdateChildAppend(node, child);
				*update_ptr=child_append;
				update_ptr=&child_append->m_Next;
				}
			break;
			}
		case Update::ChildRemove:
			{
			UINT child=0;
			size+=Dwarf::ReadUnsigned(stream, &child);
			node->m_Children.remove(child);
			if(update_ptr)
				{
				auto child_remove=new NodeUpdateChildRemove(node, child);
				*update_ptr=child_remove;
				update_ptr=&child_remove->m_Next;
				}
			break;
			}
		case Update::Clear:
			{
			node->m_Attributes.clear();
			node->m_Children.clear();
			node->m_Value=nullptr;
			if(update_ptr)
				{
				auto clear=new NodeUpdateClear(node);
				*update_ptr=clear;
				update_ptr=&clear->m_Next;
				}
			break;
			}
		case Update::TagSet:
			{
			Handle<String> tag;
			size+=tag.ReadFromStream(stream);
			node->m_Tag=tag;
			if(update_ptr)
				{
				auto tag_set=new NodeUpdateTagSet(node, tag);
				*update_ptr=tag_set;
				update_ptr=&tag_set->m_Next;
				}
			break;
			}
		case Update::ValueSet:
			{
			Handle<String> value;
			size+=value.ReadFromStream(stream);
			node->m_Value=value;
			if(update_ptr)
				{
				auto value_set=new NodeUpdateValueSet(node, value);
				*update_ptr=value_set;
				update_ptr=&value_set->m_Next;
				}
			break;
			}
		default:
			{
			throw InvalidArgumentException();
			}
		}
	}
return size;
}

SIZE_T NodeUpdate::WriteToStream(OutputStream* stream, Node* node)
{
SIZE_T size=0;
if(node->m_Tag)
	size+=NodeUpdateTagSet::WriteToStream(stream, node->m_Tag);
for(auto const& attr: node->m_Attributes)
	size+=NodeUpdateAttributeSet::WriteToStream(stream, attr.get_key(), attr.get_value());
if(node->m_Value)
	{
	size+=NodeUpdateValueSet::WriteToStream(stream, node->m_Value);
	}
else
	{
	for(auto const& child: node->m_Children)
		size+=NodeUpdateChildAppend::WriteToStream(stream, child);
	}
auto update=Update::NodeEnd;
size+=stream->Write(&update, sizeof(Update));
return size;
}


//==================
// Attribute-Remove
//==================

NodeUpdateAttributeRemove::NodeUpdateAttributeRemove(Node* node, Handle<String> key):
NodeUpdate(node),
m_Key(key)
{}

VOID NodeUpdateAttributeRemove::Create(Node* node, Handle<String> key)
{
auto update_ptr=GetUpdate(node);
while(*update_ptr)
	{
	auto update=*update_ptr;
	auto next_ptr=GetNext(update);
	auto attr_set=dynamic_cast<NodeUpdateAttributeSet*>(update);
	if(attr_set)
		{
		if(attr_set->GetKey()==key)
			{
			*update_ptr=*next_ptr;
			delete update;
			continue;
			}
		}
	update_ptr=next_ptr;
	}
*update_ptr=new NodeUpdateAttributeRemove(node, key);
}

SIZE_T NodeUpdateAttributeRemove::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto update=Update::AttributeRemove;
size+=stream->Write(&update, sizeof(Update));
size+=m_Key.WriteToStream(stream);
return size;
}


//===============
// Attribute-Set
//===============

NodeUpdateAttributeSet::NodeUpdateAttributeSet(Node* node, Handle<String> key, Handle<String> value):
NodeUpdate(node),
m_Key(key),
m_Value(value)
{}

VOID NodeUpdateAttributeSet::Create(Node* node, Handle<String> key, Handle<String> value)
{
auto update_ptr=GetUpdate(node);
while(*update_ptr)
	{
	auto update=*update_ptr;
	auto next_ptr=GetNext(update);
	auto attr_remove=dynamic_cast<NodeUpdateAttributeRemove*>(update);
	if(attr_remove)
		{
		if(attr_remove->GetKey()==key)
			{
			*update_ptr=*next_ptr;
			delete update;
			continue;
			}
		}
	auto attr_set=dynamic_cast<NodeUpdateAttributeSet*>(update);
	if(attr_set)
		{
		if(attr_set->GetKey()==key)
			{
			attr_set->SetValue(value);
			return;
			}
		}
	update_ptr=next_ptr;
	}
*update_ptr=new NodeUpdateAttributeSet(node, key, value);
}

SIZE_T NodeUpdateAttributeSet::WriteToStream(OutputStream* stream)
{
return WriteToStream(stream, m_Key, m_Value);
}

SIZE_T NodeUpdateAttributeSet::WriteToStream(OutputStream* stream, Handle<String> key, Handle<String> value)
{
SIZE_T size=0;
auto update=Update::AttributeSet;
size+=stream->Write(&update, sizeof(Update));
size+=key.WriteToStream(stream);
size+=value.WriteToStream(stream);
return size;
}


//==============
// Child-Append
//==============

NodeUpdateChildAppend::NodeUpdateChildAppend(Node* node, UINT child):
NodeUpdate(node),
m_Child(child)
{}

VOID NodeUpdateChildAppend::Create(Node* node, UINT child)
{
auto update_ptr=AppendUpdate(node);
*update_ptr=new NodeUpdateChildAppend(node, child);
}

SIZE_T NodeUpdateChildAppend::WriteToStream(OutputStream* stream)
{
return WriteToStream(stream, m_Child);
}

SIZE_T NodeUpdateChildAppend::WriteToStream(OutputStream* stream, UINT child)
{
SIZE_T size=0;
auto update=Update::ChildAppend;
size+=stream->Write(&update, sizeof(Update));
size+=Dwarf::WriteUnsigned(stream, child);
return size;
}


//==============
// Child-Remove
//==============

NodeUpdateChildRemove::NodeUpdateChildRemove(Node* node, UINT child):
NodeUpdate(node),
m_Child(child)
{}

VOID NodeUpdateChildRemove::Create(Node* node, UINT child)
{
auto update_ptr=AppendUpdate(node);
*update_ptr=new NodeUpdateChildRemove(node, child);
}

SIZE_T NodeUpdateChildRemove::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto update=Update::ChildRemove;
size+=stream->Write(&update, sizeof(Update));
size+=Dwarf::WriteUnsigned(stream, m_Child);
return size;
}


//=======
// Clear
//=======

NodeUpdateClear::NodeUpdateClear(Node* node):
NodeUpdate(node)
{}

VOID NodeUpdateClear::Create(Node* node)
{
auto update_ptr=GetUpdate(node);
while(*update_ptr)
	{
	auto update=*update_ptr;
	auto next_ptr=GetNext(update);
	auto tag_set=dynamic_cast<NodeUpdateTagSet*>(update);
	if(!tag_set)
		{
		*update_ptr=*next_ptr;
		delete update;
		continue;
		}
	update_ptr=next_ptr;
	}
*update_ptr=new NodeUpdateClear(node);
}

SIZE_T NodeUpdateClear::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto update=Update::Clear;
size+=stream->Write(&update, sizeof(Update));
return size;
}


//=========
// Tag-Set
//=========

NodeUpdateTagSet::NodeUpdateTagSet(Node* node, Handle<String> tag):
NodeUpdate(node),
m_Tag(tag)
{}

VOID NodeUpdateTagSet::Create(Node* node, Handle<String> tag)
{
auto update_ptr=GetUpdate(node);
while(*update_ptr)
	{
	auto update=*update_ptr;
	auto next_ptr=GetNext(update);
	auto tag_set=dynamic_cast<NodeUpdateTagSet*>(update);
	if(tag_set)
		{
		tag_set->m_Tag=tag;
		return;
		}
	update_ptr=next_ptr;
	}
*update_ptr=new NodeUpdateTagSet(node, tag);
}

SIZE_T NodeUpdateTagSet::WriteToStream(OutputStream* stream)
{
return WriteToStream(stream, m_Tag);
}

SIZE_T NodeUpdateTagSet::WriteToStream(OutputStream* stream, Handle<String> tag)
{
SIZE_T size=0;
auto update=Update::TagSet;
size+=stream->Write(&update, sizeof(Update));
size+=tag.WriteToStream(stream);
return size;
}


//===========
// Value-Set
//===========

NodeUpdateValueSet::NodeUpdateValueSet(Node* node, Handle<String> value):
NodeUpdate(node),
m_Value(value)
{}

VOID NodeUpdateValueSet::Create(Node* node, Handle<String> value)
{
auto update_ptr=GetUpdate(node);
while(*update_ptr)
	{
	auto update=*update_ptr;
	auto next_ptr=GetNext(update);
	auto value_set=dynamic_cast<NodeUpdateValueSet*>(update);
	if(value_set)
		{
		value_set->m_Value=value;
		return;
		}
	update_ptr=next_ptr;
	}
*update_ptr=new NodeUpdateValueSet(node, value);
}

SIZE_T NodeUpdateValueSet::WriteToStream(OutputStream* stream)
{
return WriteToStream(stream, m_Value);
}

SIZE_T NodeUpdateValueSet::WriteToStream(OutputStream* stream, Handle<String> value)
{
SIZE_T size=0;
auto update=Update::ValueSet;
size+=stream->Write(&update, sizeof(Update));
size+=value.WriteToStream(stream);
return size;
}

}}}