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

SIZE_T NodeOperation::ReadFromStream(Node* node, InputStream* stream)
{
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
			StreamReader reader(stream);
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
			StreamReader reader(stream);
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
			size+=child->ReadFromStream(stream);
			selected->AppendChildInternal(child);
			break;
			}
		case Operation::ChildInsert:
			{
			UINT pos=0;
			size+=Dwarf::ReadUnsigned(stream, &pos);
			auto child=Node::Create(selected->m_Database);
			size+=child->ReadFromStream(stream);
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
			StreamReader reader(stream);
			auto tag=reader.ReadString(&size);
			selected->SetTagInternal(tag);
			break;
			}
		case Operation::ValueSet:
			{
			StreamReader reader(stream);
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


//============
// Operations
//============

SIZE_T NodeOperationAttributeRemove::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto op=Operation::AttributeRemove;
size+=stream->Write(&op, sizeof(Operation));
size+=Dwarf::WriteUnsigned(stream, m_Position);
return size;
}

SIZE_T NodeOperationAttributeSet::WriteToStream(OutputStream* stream)
{
StreamWriter writer(stream);
SIZE_T size=0;
auto op=Operation::AttributeSet;
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
size+=writer.WriteString(m_Value);
return size;
}

SIZE_T NodeOperationAttributeSetInteger::WriteToStream(OutputStream* stream)
{
StreamWriter writer(stream);
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

SIZE_T NodeOperationChildAppend::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto op=Operation::ChildAppend;
size+=stream->Write(&op, sizeof(Operation));
size+=m_Child->WriteToStream(stream);
return size;
}

SIZE_T NodeOperationChildInsert::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto op=Operation::ChildInsert;
size+=stream->Write(&op, sizeof(Operation));
size+=Dwarf::WriteUnsigned(stream, m_Position);
size+=m_Child->WriteToStream(stream);
return size;
}

SIZE_T NodeOperationChildRemove::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto op=Operation::ChildRemove;
size+=stream->Write(&op, sizeof(Operation));
size+=Dwarf::WriteUnsigned(stream, m_Position);
return size;
}

SIZE_T NodeOperationChildSelect::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto op=Operation::ChildSelect;
size+=stream->Write(&op, sizeof(Operation));
for(auto it=m_Position->First(); it->HasCurrent(); it->MoveNext())
	size+=Dwarf::WriteUnsigned(stream, it->GetCurrent());
return size;
}

SIZE_T NodeOperationClear::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto op=Operation::Clear;
size+=stream->Write(&op, sizeof(Operation));
return size;
}

SIZE_T NodeOperationTagSet::WriteToStream(OutputStream* stream)
{
StreamWriter writer(stream);
SIZE_T size=0;
auto op=Operation::TagSet;
size+=stream->Write(&op, sizeof(Operation));
size+=writer.WriteString(m_Tag);
return size;
}

SIZE_T NodeOperationValueSet::WriteToStream(OutputStream* stream)
{
StreamWriter writer(stream);
SIZE_T size=0;
auto op=Operation::ValueSet;
size+=stream->Write(&op, sizeof(Operation));
size+=writer.WriteString(m_Value);
return size;
}

}}