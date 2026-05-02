//===================
// NodeOperation.cpp
//===================

#include "NodeOperation.h"


//=======
// Using
//=======

#include "Storage/Database/Node.h"
#include "Storage/Encoding/Dwarf.h"
#include "Storage/Streams/StreamReader.h"
#include "Storage/Streams/StreamWriter.h"

using namespace Storage::Encoding;
using namespace Storage::Streams;


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//===========
// Operation
//===========

enum class Operation: BYTE
{
None=0,
AttributeRemove,
AttributeSet,
AttributeSetInteger,
ChildAppend,
ChildInsert,
ChildRemove,
ChildSelect,
Clear,
TagSet,
ValueSet,
Done=255
};


//================
// Node-Operation
//================

SIZE_T NodeOperation::ReadFromStream(StreamReader& reader, Node* node)
{
auto stream=reader.GetStream();
SIZE_T size=0;
Node* selected=node;
while(stream->Available())
	{
	Operation op=Operation::None;
	size+=stream->Read(&op, sizeof(Operation));
	if(op==Operation::Done)
		{
		size--;
		break;
		}
	if(op==Operation::None)
		continue;
	switch(op)
		{
		case Operation::AttributeRemove:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			selected->RemoveAttributeInternal(pos);
			break;
			}
		case Operation::AttributeSet:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			if(pos==0)
				{
				auto key=reader.ReadString(&size);
				auto value=reader.ReadString(&size);
				selected->SetAttributeInternal(key, value);
				}
			else
				{
				auto value=reader.ReadString(&size);
				selected->SetAttributeInternal(pos-1, value);
				}
			return size;
			}
		case Operation::AttributeSetInteger:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			if(pos==0)
				{
				auto key=reader.ReadString(&size);
				INT64 value=0;
				size+=Dwarf::ReadSigned(stream, &value);
				selected->SetAttributeInternal(key, String::Create("%i", value));
				}
			else
				{
				INT64 value=0;
				size+=Dwarf::ReadSigned(stream, &value);
				selected->SetAttributeInternal(pos-1, String::Create("%i", value));
				}
			return size;
			}
		case Operation::ChildAppend:
			{
			auto child=Node::Create(selected->m_Database);
			size+=ReadFromStream(reader, child);
			selected->AppendChildInternal(child);
			break;
			}
		case Operation::ChildInsert:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			auto child=Node::Create(selected->m_Database);
			size+=ReadFromStream(reader, child);
			selected->InsertChildInternal(pos, child);
			break;
			}
		case Operation::ChildRemove:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			selected->RemoveChildInternal(pos);
			break;
			}
		case Operation::ChildSelect:
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
		case Operation::Clear:
			{
			selected->ClearInternal();
			break;
			}
		case Operation::TagSet:
			{
			auto tag=reader.ReadString(&size);
			selected->SetTagInternal(tag);
			break;
			}
		case Operation::ValueSet:
			{
			auto value=reader.ReadString(&size);
			selected->SetValueInternal(value);
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

SIZE_T NodeOperation::WriteToStream(StreamWriter& writer, Node* node)
{
auto stream=writer.GetStream();
SIZE_T size=0;
size+=NodeOperationTagSet::WriteToStream(writer, node->m_Tag);
for(auto const& it: node->m_Attributes)
	{
	auto const& key=it.get_key();
	auto const& value=it.get_value();
	size+=NodeOperationAttributeSet::WriteToStream(writer, 0, key, value);
	}
auto const& value=node->m_Value;
if(value)
	{
	size+=NodeOperationValueSet::WriteToStream(writer, value);
	}
else
	{
	for(auto const& child: node->m_Children)
		size+=NodeOperationChildAppend::WriteToStream(writer, child.As<Node>());
	}
if(size%2)
	{
	auto op=Operation::None;
	size+=stream->Write(&op, sizeof(Operation));
	}
return size;
}


//============
// Operations
//============

SIZE_T NodeOperationAttributeRemove::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::AttributeRemove;
size+=stream->Write(&op, sizeof(Operation));
size+=Dwarf::WriteUnsigned(stream, m_Position);
return size;
}

SIZE_T NodeOperationAttributeSet::WriteToStream(StreamWriter& writer, UINT pos, Handle<String> key, Handle<String> value)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::AttributeSet;
size+=stream->Write(&op, sizeof(Operation));
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

SIZE_T NodeOperationAttributeSetInteger::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::AttributeSetInteger;
size+=stream->Write(&op, sizeof(Operation));
if(m_Key)
	{
	size+=Dwarf::WriteUnsigned(stream, m_Position);
	size+=writer.WriteString(m_Key);
	}
else
	{
	size+=Dwarf::WriteUnsigned(stream, m_Position+1);
	}
size+=Dwarf::WriteSigned(stream, m_Value);
return size;
}

SIZE_T NodeOperationChildAppend::WriteToStream(StreamWriter& writer, Node* child)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::ChildAppend;
size+=stream->Write(&op, sizeof(Operation));
size+=NodeOperation::WriteToStream(writer, child);
return size;
}

SIZE_T NodeOperationChildInsert::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::ChildInsert;
size+=stream->Write(&op, sizeof(Operation));
size+=Dwarf::WriteUnsigned(stream, m_Position);
size+=m_Child->WriteToStream(stream);
return size;
}

SIZE_T NodeOperationChildRemove::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::ChildRemove;
size+=stream->Write(&op, sizeof(Operation));
size+=Dwarf::WriteUnsigned(stream, m_Position);
return size;
}

SIZE_T NodeOperationChildSelect::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::ChildSelect;
size+=stream->Write(&op, sizeof(Operation));
for(auto it=m_Position->First(); it->HasCurrent(); it->MoveNext())
	size+=Dwarf::WriteUnsigned(stream, it->GetCurrent());
return size;
}

SIZE_T NodeOperationClear::WriteToStream(StreamWriter& writer)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::Clear;
size+=stream->Write(&op, sizeof(Operation));
return size;
}

SIZE_T NodeOperationTagSet::WriteToStream(StreamWriter& writer, Handle<String> tag)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::TagSet;
size+=stream->Write(&op, sizeof(Operation));
size+=writer.WriteString(tag);
return size;
}

SIZE_T NodeOperationValueSet::WriteToStream(StreamWriter& writer, Handle<String> value)
{
auto stream=writer.GetStream();
SIZE_T size=0;
auto op=Operation::ValueSet;
size+=stream->Write(&op, sizeof(Operation));
size+=writer.WriteString(value);
return size;
}

}}