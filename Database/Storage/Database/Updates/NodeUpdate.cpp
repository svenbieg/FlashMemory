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

NodeUpdate** NodeUpdate::GetUpdate(Node* node)
{
return &node->m_Update;
}

SIZE_T NodeUpdate::ReadFromStream(InputStream* stream, Node* node)
{
SIZE_T size=0;
Update update=Update::None;
while(stream->Available())
	{
	size+=stream->Read(&update, sizeof(Update));
	if(update==Update::NodeBegin)
		break;
	}
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
			break;
			}
		case Update::AttributeSet:
			{
			Handle<String> key;
			size+=key.ReadFromStream(stream);
			Handle<String> value;
			size+=value.ReadFromStream(stream);
			node->m_Attributes.set(key, value);
			return size;
			}
		case Update::ChildAppend:
			{
			UINT child=0;
			size+=Dwarf::ReadUnsigned(stream, &child);
			node->m_Children.append(child);
			break;
			}
		case Update::ChildRemove:
			{
			UINT child=0;
			size+=Dwarf::ReadUnsigned(stream, &child);
			node->m_Children.remove(child);
			break;
			}
		case Update::Clear:
			{
			node->m_Attributes.clear();
			node->m_Children.clear();
			node->m_Value=nullptr;
			break;
			}
		case Update::TagSet:
			{
			Handle<String> tag;
			size+=tag.ReadFromStream(stream);
			node->m_Tag=tag;
			break;
			}
		case Update::ValueSet:
			{
			Handle<String> value;
			size+=value.ReadFromStream(stream);
			node->m_Value=value;
			break;
			}
		default:
			{
			throw InvalidArgumentException();
			}
		}
	}
FlagHelper::Set(node->m_Flags, Node::NodeFlags::Update);
return size;
}

SIZE_T NodeUpdate::WriteToStream(OutputStream* stream, Node* node)
{
SIZE_T size=0;
auto const& tag=node->m_Tag;
if(tag)
	{
	size+=NodeUpdateTagSet::WriteToStream(stream, tag);
	}
for(auto const& it: node->m_Attributes)
	{
	auto const& key=it.get_key();
	auto const& value=it.get_value();
	size+=NodeUpdateAttributeSet::WriteToStream(stream, key, value);
	}
auto const& value=node->m_Value;
if(value)
	{
	size+=NodeUpdateValueSet::WriteToStream(stream, value);
	}
else
	{
	for(auto child: node->m_Children)
		{
		size+=NodeUpdateChildAppend::WriteToStream(stream, child);
		}
	}
auto update=Update::NodeEnd;
size+=stream->Write(&update, sizeof(Update));
if(size%2)
	{
	update=Update::None;
	size+=stream->Write(&update, sizeof(Update));
	}
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
	auto next_ptr=update->GetNext();
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
	auto next_ptr=update->GetNext();
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
auto update_ptr=GetUpdate(node);
while(*update_ptr)
	{
	auto update=*update_ptr;
	auto update_ptr=update->GetNext();
	}
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
auto update_ptr=GetUpdate(node);
while(*update_ptr)
	{
	auto update=*update_ptr;
	auto update_ptr=update->GetNext();
	}
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
	auto next_ptr=update->GetNext();
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
	auto next_ptr=update->GetNext();
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
	auto next_ptr=update->GetNext();
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