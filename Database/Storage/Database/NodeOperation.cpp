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
			size+=NodeOperationAttributeRemove::ReadFromStream(selected, stream);
			break;
			}
		case Operation::AttributeSet:
			{
			size+=NodeOperationAttributeSet::ReadFromStream(selected, stream);
			break;
			}
		case Operation::ChildAppend:
			{
			size+=NodeOperationChildAppend::ReadFromStream(selected, stream);
			break;
			}
		case Operation::ChildInsert:
			{
			size+=NodeOperationChildInsert::ReadFromStream(selected, stream);
			break;
			}
		case Operation::ChildRemove:
			{
			size+=NodeOperationChildRemove::ReadFromStream(selected, stream);
			break;
			}
		case Operation::ChildSelect:
			{
			size+=NodeOperationChildSelect::ReadFromStream(node, stream, &selected);
			break;
			}
		case Operation::Clear:
			{
			selected->Clear();
			break;
			}
		case Operation::TagSet:
			{
			size+=NodeOperationTagSet::ReadFromStream(selected, stream);
			break;
			}
		case Operation::ValueSet:
			{
			size+=NodeOperationValueSet::ReadFromStream(selected, stream);
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


//==================
// Attribute-Remove
//==================

SIZE_T NodeOperationAttributeRemove::ReadFromStream(Node* target, InputStream* stream)
{
StreamReader reader(stream);
SIZE_T size=0;
auto key=reader.ReadString(&size);
target->RemoveAttribute(key);
return size;
}

SIZE_T NodeOperationAttributeRemove::WriteToStream(OutputStream* stream)
{
StreamWriter writer(stream);
SIZE_T size=0;
auto op=Operation::AttributeRemove;
size+=stream->Write(&op, sizeof(Operation));
size+=writer.WriteString(m_Key);
return size;
}


//===============
// Attribute-Set
//===============

SIZE_T NodeOperationAttributeSet::ReadFromStream(Node* target, InputStream* stream)
{
StreamReader reader(stream);
SIZE_T size=0;
auto key=reader.ReadString(&size);
target->RemoveAttribute(key);
return size;
}

SIZE_T NodeOperationAttributeSet::WriteToStream(OutputStream* stream)
{
StreamWriter writer(stream);
SIZE_T size=0;
auto op=Operation::AttributeSet;
size+=stream->Write(&op, sizeof(Operation));
size+=writer.WriteString(m_Key);
size+=writer.WriteString(m_Value);
return size;
}


//==============
// Child-Append
//==============

SIZE_T NodeOperationChildAppend::ReadFromStream(Node* target, InputStream* stream)
{
SIZE_T size=0;
auto child=Node::Create();
size+=child->ReadFromStream(stream);
target->AppendChild(child);
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


//==============
// Child-Insert
//==============

SIZE_T NodeOperationChildInsert::ReadFromStream(Node* target, InputStream* stream)
{
SIZE_T size=0;
UINT pos=0;
size+=Dwarf::ReadUnsigned(stream, &pos);
auto child=Node::Create();
size+=child->ReadFromStream(stream);
target->InsertChildAt(pos, child);
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


//==============
// Child-Remove
//==============

SIZE_T NodeOperationChildRemove::ReadFromStream(Node* target, InputStream* stream)
{
SIZE_T size=0;
UINT pos=0;
size+=Dwarf::ReadUnsigned(stream, &pos);
target->RemoveChildAt(pos);
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


//==============
// Child-Select
//==============

SIZE_T NodeOperationChildSelect::ReadFromStream(Node* target, InputStream* stream, Node** selected)
{
SIZE_T size=0;
Node* node=target;
while(1)
	{
	UINT pos=0;
	size+=Dwarf::ReadUnsigned(stream, &pos);
	if(pos==0)
		break;
	auto child=node->GetChildAt(pos-1);
	node=child.As<Node>();
	}
*selected=node;
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


//=======
// Clear
//=======

SIZE_T NodeOperationClear::WriteToStream(OutputStream* stream)
{
SIZE_T size=0;
auto op=Operation::Clear;
size+=stream->Write(&op, sizeof(Operation));
return size;
}


//=========
// Tag-Set
//=========

SIZE_T NodeOperationTagSet::ReadFromStream(Node* target, InputStream* stream)
{
StreamReader reader(stream);
SIZE_T size=0;
auto tag=reader.ReadString(&size);
target->SetTag(tag);
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


//===========
// Value-Set
//===========

SIZE_T NodeOperationValueSet::ReadFromStream(Node* target, InputStream* stream)
{
StreamReader reader(stream);
SIZE_T size=0;
auto value=reader.ReadString(&size);
target->SetValue(value);
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