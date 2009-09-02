/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2008-10-01 11:54:41 +0200 (Mi, 01 Okt 2008) $
 Version:   $Revision: 15350 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryXMLMemento.h"

#include "internal/cherryWorkbenchPlugin.h"

#include "Poco/NumberParser.h"
#include "Poco/DOM/NodeList.h" 
#include "Poco/XML/NamePool.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/Attr.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/DOMBuilder.h"
#include "Poco/SAX/InputSource.h"

#include <sstream>

const std::string EMPTY_STRING;

cherry::XMLMemento::XMLMemento(Poco::XML::Document* document, Poco::XML::Element* elem) 
{
  factory = document;
  element = elem;
}

cherry::XMLMemento::Pointer cherry::XMLMemento::CreateReadRoot(cherry::XMLMemento::XMLByteInputStream& reader) throw(WorkbenchException)
{
  return CreateReadRoot(reader, "");
}

cherry::XMLMemento::Pointer cherry::XMLMemento::CreateReadRoot(cherry::XMLMemento::XMLByteInputStream& reader, const std::string& baseDir) throw(WorkbenchException)
{
    //TODO try catch blog
    
      Poco::XML::NamePool* namePool = new Poco::XML::NamePool();  
      Poco::XML::Document* doc = new Poco::XML::Document(namePool);
      Poco::XML::DOMParser* parser = new Poco::XML::DOMParser();
      Poco::XML::InputSource* source = new Poco::XML::InputSource(reader);


      source->setSystemId(baseDir);
      doc = parser->parse(source);

      Poco::XML::Element* elem = doc->documentElement(); //TODO really documentElement?

      return cherry::XMLMemento::New(doc, elem);
}

cherry::XMLMemento::Pointer cherry::XMLMemento::CreateWriteRoot(const std::string& type) 
{
// TODO
//  try{
    Poco::XML::NamePool* namePool = new Poco::XML::NamePool();  
    Poco::XML::Document* doc = new Poco::XML::Document(namePool);
    Poco::XML::Element* elem = doc->createElement(type);
    doc->appendChild(elem);
    

    return cherry::XMLMemento::New(doc, elem);
  //}catch() //TODO: look for poco exceptions
  //{
  //}
}

cherry::IMemento::Pointer cherry::XMLMemento::CreateChild(const std::string& type)
{
  Poco::XML::Element* child = factory->createElement(type);
  element->appendChild(child);
  return XMLMemento::New(factory, child);
}

cherry::IMemento::Pointer cherry::XMLMemento::CreateChild(const std::string& type, const std::string& id)
{
  Poco::XML::Element* child = factory->createElement(type);
  child->setAttribute(TAG_ID, id); //$NON-NLS-1$
  element->appendChild(child);
  return XMLMemento::New(factory, child);
}

cherry::IMemento::Pointer cherry::XMLMemento::CopyChild(IMemento::Pointer child) 
{
  //TODO check any casting errors
  Poco::XML::Element* elem = child.Cast<cherry::XMLMemento>()->GetElement();
  Poco::XML::Element* newElement = dynamic_cast<Poco::XML::Element*>(factory->importNode(elem,true));
  element->appendChild(newElement);
  return XMLMemento::New(factory, newElement);

}

cherry::IMemento::Pointer cherry::XMLMemento::GetChild(const std::string& type) const
{
  // Get the nodes.
  cherry::XMLMemento::Pointer memento;
  Poco::XML::Element* child = element->getChildElement(type); // Find the first node which is a child of this node
  if (child)
  {
    memento = cherry::XMLMemento::New(factory, child);
    return memento;
  }
  // A child was not found.
  return memento;
}

std::vector< cherry::IMemento::Pointer > cherry::XMLMemento::GetChildren(const std::string& type) const
{
  std::vector< IMemento::Pointer > mementos;
  Poco::XML::NodeList* elementList = element->getElementsByTagName(type);
  mementos.resize(elementList->length());
  for (unsigned long i = 0; i < elementList->length(); i++)
  {
    Poco::XML::Element*  elem = dynamic_cast<Poco::XML::Element*>(elementList->item(i));
    mementos[i] = cherry::XMLMemento::New(factory, elem);
  }

  return mementos;

}


bool cherry::XMLMemento::GetFloat(const std::string& key, double& value) const
{
  const std::string& attr = element->getAttribute(key);

  try {
    value = Poco::NumberParser::parseFloat(attr);
  } catch (const Poco::SyntaxException& e) {
    WorkbenchPlugin::Log("Memento problem - invalid integer for key: " + key
                            + " value: " + attr, e);
    return false;
  }

  return true;
}

std::string cherry::XMLMemento::GetType() const
{
  return element->nodeName();
}

std::string cherry::XMLMemento::GetID() const
{
  //TODO: make error handling!
  return element->getAttribute(TAG_ID);
}

bool cherry::XMLMemento::GetInteger(const std::string& key, int& value) const
{
  const std::string& attr = element->getAttribute(key);

  try
  {
    value = Poco::NumberParser::parse(attr);
  }
  catch (const Poco::SyntaxException& e) {
     WorkbenchPlugin::Log("Memento problem - invalid integer for key: " + key
                            + " value: " + attr, e);
     return false;
  }

  return true;
}

bool cherry::XMLMemento::GetBoolean(const std::string& key, bool& value) const
{
  std::string attr = element->getAttribute(key);
  if (attr.empty()) return false;
  else if (attr == "true")
  {
    value = true;
    return true;
  }
  else
  {
    value = false;
    return true;
  }
}

bool cherry::XMLMemento::GetString(const std::string& key, std::string& value) const
{
  std::string v = element->getAttribute(key);
  if (v.empty()) return false;

  value = v;
  return true;
}

const std::string& cherry::XMLMemento::GetTextData() const
{
  Poco::XML::Text* textNode = GetTextNode();

  if (textNode != NULL)
  {
    return textNode->getData();
  }

  return EMPTY_STRING; //TODO check if NULL or better ""

}

std::vector< std::string > cherry::XMLMemento::GetAttributeKeys() const
{
  std::vector < std::string > values;
  Poco::XML::NamedNodeMap* nnMap = element->attributes();

  values.resize( nnMap->length() );
  
  for (unsigned long i = 0; i < nnMap->length(); i++)
  {
    values[i] = nnMap->item(i)->nodeName(); //TODO check if right
  }

  return values;
}

Poco::XML::Text* cherry::XMLMemento::GetTextNode()  const
{

  //Get the nodes
  Poco::XML::NodeList* nodes = element->childNodes();

  unsigned long size = nodes->length();
  
  if (size == 0)
    return NULL;

  //Search for the text node
  for (unsigned long index = 0; index < size; index++)
  {
    if (nodes->item(index)->nodeType() == Poco::XML::Node::TEXT_NODE)
    {
      return dynamic_cast<Poco::XML::Text*>(nodes->item(index));
    }
  }

  // a Text node was not found
  return NULL;

}

void cherry::XMLMemento::PutElement(Poco::XML::Element* element, bool copyText)
{
  Poco::XML::NodeList* nodeAttr = dynamic_cast<Poco::XML::NodeList*>(element->attributes());
  unsigned long size = nodeAttr->length();

  for (unsigned long index = 0; index < size; index++)
  {
    if (nodeAttr->item(index)->nodeType() == Poco::XML::Node::ATTRIBUTE_NODE)
    {
      Poco::XML::Attr* attr = dynamic_cast<Poco::XML::Attr*>(nodeAttr->item(index));
      PutString(attr->nodeName(),attr->nodeValue());
    }
  }

  Poco::XML::NodeList* nodes = dynamic_cast<Poco::XML::NodeList*>(element->attributes());
  size = nodes->length();

  bool needToCopyText = copyText;
  for (unsigned long index = 0; index < size; index++)
  {
    unsigned short nodeType = nodes->item(index)->nodeType();
    switch(nodeType)
    {
    case Poco::XML::Node::ELEMENT_NODE:
      {
        Poco::XML::Element* elem = dynamic_cast<Poco::XML::Element*>(nodes->item(index));

        cherry::XMLMemento::Pointer child = CreateChild(elem->nodeName()).Cast<cherry::XMLMemento>();
        child->PutElement(elem,true);
      }
      break;

    case Poco::XML::Node::TEXT_NODE:
      if (needToCopyText)
      {
        Poco::XML::Text* text = dynamic_cast<Poco::XML::Text*>(nodes->item(index));
        PutTextData(text->getData());
        needToCopyText = false;
      }
      break;

    default:
      break;
    }
  }
}

void cherry::XMLMemento::PutFloat(const std::string& key, float value)
{
  std::stringstream ss;
  std::string xmlValue;

  ss << value;
  ss >> xmlValue;
  element->setAttribute(key, xmlValue);
  //element.setAttribute(key, String.valueOf(f));
}

void cherry::XMLMemento::PutInteger(const std::string& key, int value) 
{
  std::stringstream ss;
  std::string xmlValue;

  ss << value;
  ss >> xmlValue;
  element->setAttribute(key, xmlValue);
  //element.setAttribute(key, String.valueOf(n));
}

void cherry::XMLMemento::PutMemento(IMemento::Pointer memento)
{
  // Do not copy the element's top level text node (this would overwrite the existing text).
  // Text nodes of children are copied.
  //TODO check cast throw cast error
  PutElement(memento.Cast<cherry::XMLMemento>()->GetElement(), false);
}

void cherry::XMLMemento::PutString(const std::string& key, const std::string& value)
{
  element->setAttribute(key, value);
  //if (value == null) {
  //  return;}
  //element.setAttribute(key, value);
}

void cherry::XMLMemento::PutBoolean(const std::string& key, bool value)
{
  if (value)
  {
    element->setAttribute(key, "true");
  }
  else
  {
    element->setAttribute(key, "false");
  }
}

void cherry::XMLMemento::PutTextData(const std::string& data) 
{
  Poco::XML::Text* textNode = GetTextNode();
  if (textNode == NULL)
  {
    textNode = factory->createTextNode(data);
    element->insertBefore(textNode, element->firstChild());
  }
  else
  {
    textNode->setData(data);
  }
}

void cherry::XMLMemento::Save(cherry::XMLMemento::XMLByteOutputStream& writer)
{
  if (writer.good())
  {
    Poco::XML::DOMWriter* out = new Poco::XML::DOMWriter();
    out->setOptions(3); //write declaration and pretty print
    out->writeNode(writer, factory);
    delete out;
  }
  else
  {
    //TODO
  }
}
Poco::XML::Element* cherry::XMLMemento::GetElement() const
{
  return element;
}

