//========
// Node.h
//========

// Copyright 2026, Sven Bieg (svenbieg@outlook.de)
// https://github.com/svenbieg/Database

#pragma once


//=======
// Using
//=======

#include "Storage/Xml/XmlNode.h"


//===========
// Namespace
//===========

namespace Storage {
	namespace Database {


//======
// Node
//======

class Node: public Xml::XmlNode
{
public:
	// Friends
	friend Object;

	// Con-/Destructors
	static inline Handle<Node> Create(Handle<String> Tag=nullptr) { return Object::Create<Node>(Tag); }

	// XML-Node
	SIZE_T ReadFromStream(InputStream* Stream)override;
	SIZE_T WriteToStream(OutputStream* Stream, INT Level=-1)override;

protected:
	// Settings
	static const UINT NODE_ID='NODE';

	// Con-/Destructors
	Node(Handle<String> Tag);
};

}}