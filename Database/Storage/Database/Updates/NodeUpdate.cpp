//================
// NodeUpdate.cpp
//================

#include "NodeUpdate.h"


//=======
// Using
//=======

#include "Storage/Database/Node.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"
#include "FlagHelper.h"

using namespace Storage::Encoding;
using namespace Storage::Streams;


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
AttributeSetInt64,
ChildAppend,
ChildRemove,
ChildSelect,
Clear,
NodeEnd,
TagSet,
ValueSet
};


//=============
// Node-Update
//=============

SIZE_T NodeUpdate::ReadFromStream(StreamReader& reader, Node* node)
{
auto stream=reader.GetStream();
SIZE_T size=0;
Node* selected=node;
while(stream->Available())
	{
	Update update=Update::None;
	size+=stream->Read(&update, sizeof(Update));
	if(update==Update::None)
		continue;
	if(update==Update::NodeEnd)
		break;
	switch(update)
		{
		case Update::AttributeRemove:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			selected->m_Attributes.remove_at(pos);
			break;
			}
		case Update::AttributeSet:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			if(pos==0)
				{
				auto key=reader.ReadString(&size);
				auto value=reader.ReadString(&size);
				selected->m_Attributes.set(key, value);
				}
			else
				{
				auto& attr=selected->m_Attributes.get_at(pos-1);
				auto value=reader.ReadString(&size);
				attr.set_value(value);
				}
			return size;
			}
		case Update::AttributeSetInt64:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			if(pos==0)
				{
				auto key=reader.ReadString(&size);
				INT64 value=0;
				size+=Dwarf::ReadSigned(stream, &value);
				selected->m_Attributes.set(key, String::Create("%i", value));
				}
			else
				{
				auto& attr=selected->m_Attributes.get_at(pos-1);
				INT64 value=0;
				size+=Dwarf::ReadSigned(stream, &value);
				attr.set_value(String::Create("%i", value));
				}
			return size;
			}
		case Update::ChildAppend:
			{
			auto child=Node::Create(selected);
			size+=ReadFromStream(reader, child);
			break;
			}
		case Update::ChildRemove:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			selected->m_Children.remove_at(pos);
			break;
			}
		case Update::ChildSelect:
			{
			selected=node;
			while(1)
				{
				UINT pos=0;
				size+=Dwarf::ReadUnsigned(stream, &pos);
				if(pos==0)
					break;
				auto child=node->GetChildAt(pos-1);
				selected=child.As<Node>();
				}
			break;
			}
		case Update::Clear:
			{
			selected->m_Attributes.clear();
			selected->m_Children.clear();
			selected->m_Value=nullptr;
			break;
			}
		case Update::TagSet:
			{
			auto tag=reader.ReadString(&size);
			selected->m_Tag=tag;
			break;
			}
		case Update::ValueSet:
			{
			auto value=reader.ReadString(&size);
			selected->m_Value=value;
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

SIZE_T NodeUpdate::WriteToStream(StreamWriter& writer, Node* node)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto const& tag=node->m_Tag;
if(tag)
	{
	size+=NodeUpdateTagSet::WriteToStream(writer, tag);
	}
for(auto const& it: node->m_Attributes)
	{
	auto const& key=it.get_key();
	auto const& value=it.get_value();
	size+=NodeUpdateAttributeSet::WriteToStream(writer, 0, key, value);
	}
auto const& value=node->m_Value;
if(value)
	{
	size+=NodeUpdateValueSet::WriteToStream(writer, value);
	}
else
	{
	for(auto const& child: node->m_Children)
		{
		size+=NodeUpdateChildAppend::WriteToStream(writer, child);
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


//=========
// Updates
//=========

SIZE_T NodeUpdateAttributeRemove::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::AttributeRemove;
size+=stream->Write(&op, sizeof(Update));
size+=Dwarf::WriteUnsigned(stream, m_Position);
return size;
}

SIZE_T NodeUpdateAttributeSet::WriteToStream(StreamWriter& writer, UINT pos, Handle<String> key, Handle<String> value)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::AttributeSet;
size+=stream->Write(&op, sizeof(Update));
if(key)
	{
	size+=Dwarf::WriteUnsigned(stream, 0);
	size+=writer.WriteString(key);
	}
else
	{
	size+=Dwarf::WriteUnsigned(stream, pos+1);
	}
size+=writer.WriteString(value);
return size;
}

SIZE_T NodeUpdateAttributeSetInt64::WriteToStream(StreamWriter& writer, UINT pos, Handle<String> key, INT64 value)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::AttributeSetInt64;
size+=stream->Write(&op, sizeof(Update));
if(key)
	{
	size+=Dwarf::WriteUnsigned(stream, 0);
	size+=writer.WriteString(key);
	}
else
	{
	size+=Dwarf::WriteUnsigned(stream, pos+1);
	}
size+=Dwarf::WriteSigned(stream, value);
return size;
}

SIZE_T NodeUpdateChildAppend::WriteToStream(StreamWriter& writer, Node* child)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::ChildAppend;
size+=stream->Write(&op, sizeof(Update));
size+=NodeUpdate::WriteToStream(writer, child);
return size;
}

SIZE_T NodeUpdateChildRemove::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::ChildRemove;
size+=stream->Write(&op, sizeof(Update));
size+=Dwarf::WriteUnsigned(stream, m_Position);
return size;
}

SIZE_T NodeUpdateChildSelect::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::ChildSelect;
size+=stream->Write(&op, sizeof(Update));
for(auto it=m_Position->First(); it->HasCurrent(); it->MoveNext())
	size+=Dwarf::WriteUnsigned(stream, it->GetCurrent());
return size;
}

SIZE_T NodeUpdateClear::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::Clear;
size+=stream->Write(&op, sizeof(Update));
return size;
}

SIZE_T NodeUpdateTagSet::WriteToStream(StreamWriter& writer, Handle<String> tag)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::TagSet;
size+=stream->Write(&op, sizeof(Update));
size+=writer.WriteString(tag);
return size;
}

SIZE_T NodeUpdateValueSet::WriteToStream(StreamWriter& writer, Handle<String> value)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Update::ValueSet;
size+=stream->Write(&op, sizeof(Update));
size+=writer.WriteString(value);
return size;
}

}}}