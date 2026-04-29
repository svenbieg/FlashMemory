//==========
// Node.cpp
//==========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#include "Node.h"


//=======
// Using
//=======

#include "Storage/Database/Database.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//==========
// XML-Node
//==========

SIZE_T Node::ReadFromStream(InputStream* stream)
{
assert(stream);
UINT id=0;
SIZE_T size=stream->Read(&id, sizeof(UINT));
if(id!=NODE_ID)
	throw InvalidArgumentException();
try
	{
	size+=XmlNode::ReadFromStream(stream);
	}
catch(OutOfRangeException)
	{
	throw InvalidArgumentException();
	}
size=TypeHelper::AlignUp(size, 2U);
return size;
}

SIZE_T Node::WriteToStream(OutputStream* stream, INT level)
{
assert(stream);
SIZE_T size=0;
size+=stream->Write(&NODE_ID, sizeof(UINT));
size+=XmlNode::WriteToStream(stream, -1);
if(size%2)
	{
	BYTE zero=0;
	size+=stream->Write(&zero, 1);
	}
return size;
}


//============================
// Con-/Destructors Protected
//============================

Node::Node(Handle<String> tag):
XmlNode(tag)
{}

}}