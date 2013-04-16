//
// Text.h
//
// $Id$
//
// Library: XML
// Package: DOM
// Module:  DOM
//
// Definition of the DOM Text class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef DOM_Text_INCLUDED
#define DOM_Text_INCLUDED


#include "Poco/XML/XML.h"
#include "Poco/DOM/CharacterData.h"
#include "Poco/XML/XMLString.h"


namespace Poco {
namespace XML {


class XML_API Text: public CharacterData
	/// The Text interface inherits from CharacterData and represents the textual
	/// content (termed character data in XML) of an Element or Attr. If there is
	/// no markup inside an element's content, the text is contained in a single
	/// object implementing the Text interface that is the only child of the element.
	/// If there is markup, it is parsed into the information items (elements, comments,
	/// etc.) and Text nodes that form the list of children of the element.
	/// 
	/// When a document is first made available via the DOM, there is only one Text
	/// node for each block of text. Users may create adjacent Text nodes that represent
	/// the contents of a given element without any intervening markup, but should
	/// be aware that there is no way to represent the separations between these
	/// nodes in XML or HTML, so they will not (in general) persist between DOM
	/// editing sessions. The normalize() method on Element merges any such adjacent
	/// Text objects into a single node for each block of text.
{
public:
	Text* splitText(unsigned long offset);
		/// Breaks this node into two nodes at the specified offset, keeping both in
		/// the tree as siblings. This node then only contains all the content up to
		/// the offset point. A new node of the same type, which is inserted as the
		/// next sibling of this node, contains all the content at and after the offset
		/// point. When the offset is equal to the length of this node, the new node
		/// has no data.

	// Node
	const XMLString& nodeName() const;
	unsigned short nodeType() const;

	// Non-standard extensions
	XMLString innerText() const;

protected:
	Text(Document* pOwnerDocument, const XMLString& data);
	Text(Document* pOwnerDocument, const Text& text);
	~Text();

	Node* copyNode(bool deep, Document* pOwnerDocument) const;

private:
	static const XMLString NODE_NAME;
	
	friend class Document;
};


} } // namespace Poco::XML


#endif // DOM_Text_INCLUDED
