/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryXMLMemento.h"

#include "internal/berryWorkbenchPlugin.h"

#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/XML/NamePool.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/Text.h>
#include <Poco/DOM/Attr.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/DOMBuilder.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/SAX/SAXException.h>

#include <sstream>
#include <limits>

const std::string EMPTY_STRING;

berry::XMLMemento::XMLMemento(Poco::XML::Document* document,
    Poco::XML::Element* elem) :
  factory(document), element(elem)
{
  factory->duplicate();
  element->duplicate();
}

berry::XMLMemento::~XMLMemento()
{
  element->release();
  factory->release();
}

berry::XMLMemento::Pointer berry::XMLMemento::CreateReadRoot(
    berry::XMLMemento::XMLByteInputStream& reader) throw (WorkbenchException)
{
  return CreateReadRoot(reader, "");
}

berry::XMLMemento::Pointer berry::XMLMemento::CreateReadRoot(
    berry::XMLMemento::XMLByteInputStream& reader, const std::string& baseDir)
    throw (WorkbenchException)
{
  std::string errorMessage;
  Poco::Exception exception("");

  try
  {
    Poco::XML::DOMParser parser;
    Poco::XML::InputSource source(reader);

    source.setSystemId(baseDir);
    Poco::XML::Document* doc = parser.parse(&source);

    Poco::XML::Element* elem = doc->documentElement();

    XMLMemento::Pointer memento = XMLMemento::New(doc, elem);

    doc->release();

    return memento;
  } catch (Poco::XML::SAXParseException& e)
  {
    errorMessage = "Could not parse content of XML file.";
    exception = e;
  }

  std::string problemText = exception.message();
  if (problemText.empty())
  {
    problemText = errorMessage.empty() ? "Could not find root element node of XML file." : errorMessage;
  }
  throw WorkbenchException(problemText, exception);

}

berry::XMLMemento::Pointer berry::XMLMemento::CreateWriteRoot(
    const std::string& type)
{
  // TODO
  //  try{
  Poco::XML::Document* doc = new Poco::XML::Document();
  Poco::XML::Element* elem = doc->createElement(type);
  doc->appendChild(elem)->release();

  XMLMemento::Pointer memento = XMLMemento::New(doc, elem);
  doc->release();
  return memento;
  //}catch() //TODO: look for poco exceptions
  //{
  //}
}

berry::IMemento::Pointer berry::XMLMemento::CreateChild(
    const std::string& type)
{
  Poco::XML::Element* child = factory->createElement(type);
  element->appendChild(child)->release();
  return XMLMemento::New(factory, child);
}

berry::IMemento::Pointer berry::XMLMemento::CreateChild(
    const std::string& type, const std::string& id)
{
  Poco::XML::Element* child = factory->createElement(type);
  child->setAttribute(TAG_ID, id); //$NON-NLS-1$
  element->appendChild(child)->release();
  return XMLMemento::New(factory, child);
}

berry::IMemento::Pointer berry::XMLMemento::CopyChild(IMemento::Pointer child)
{
  //TODO check any casting errors
  Poco::XML::Element* elem = child.Cast<berry::XMLMemento> ()->GetElement();
  Poco::XML::Element* newElement =
      dynamic_cast<Poco::XML::Element*> (factory->importNode(elem, true));
  element->appendChild(newElement)->release();
  return XMLMemento::New(factory, newElement);

}

berry::IMemento::Pointer berry::XMLMemento::GetChild(const std::string& type) const
{
  // Get the nodes.
  berry::XMLMemento::Pointer memento;
  Poco::XML::Element* child = element->getChildElement(type); // Find the first node which is a child of this node
  if (child)
  {
    memento = berry::XMLMemento::New(factory, child);
    return memento;
  }
  // A child was not found.
  return memento;
}

std::vector<berry::IMemento::Pointer> berry::XMLMemento::GetChildren(
    const std::string& type) const
{
  std::vector<IMemento::Pointer> mementos;
  Poco::XML::NodeList* elementList = element->getElementsByTagName(type);
  mementos.resize(elementList->length());
  for (unsigned long i = 0; i < elementList->length(); i++)
  {
    Poco::XML::Element* elem =
        dynamic_cast<Poco::XML::Element*> (elementList->item(i));
    mementos[i] = berry::XMLMemento::New(factory, elem);
  }

  return mementos;

}

bool berry::XMLMemento::GetFloat(const std::string& key, double& value) const
{
  if (!element->hasAttribute(key)) return false;

  const std::string& attr = element->getAttribute(key);

  try
  {
    value = Poco::NumberParser::parseFloat(attr);
  } catch (const Poco::SyntaxException& e)
  {
    std::string _qnan = Poco::NumberFormatter::format(std::numeric_limits<double>::quiet_NaN());
    if (_qnan == attr)
    {
      value = std::numeric_limits<double>::quiet_NaN();
      return true;
    }

    std::string _inf = Poco::NumberFormatter::format(std::numeric_limits<double>::infinity());
    if (_inf == attr)
    {
      value = std::numeric_limits<double>::infinity();
      return true;
    }

    WorkbenchPlugin::Log("Memento problem - invalid float for key: " + key
        + " value: " + attr, e);
    return false;
  }

  return true;
}

std::string berry::XMLMemento::GetType() const
{
  return element->nodeName();
}

std::string berry::XMLMemento::GetID() const
{
  //TODO: make error handling!
  return element->getAttribute(TAG_ID);
}

bool berry::XMLMemento::GetInteger(const std::string& key, int& value) const
{
  if (!element->hasAttribute(key)) return false;

  const std::string& attr = element->getAttribute(key);

  try
  {
    value = Poco::NumberParser::parse(attr);
  } catch (const Poco::SyntaxException& e)
  {
    WorkbenchPlugin::Log("Memento problem - invalid integer for key: " + key
        + " value: " + attr, e);
    return false;
  }

  return true;
}

bool berry::XMLMemento::GetBoolean(const std::string& key, bool& value) const
{
  std::string attr = element->getAttribute(key);
  if (attr.empty())
    return false;
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

bool berry::XMLMemento::GetString(const std::string& key, std::string& value) const
{
  std::string v = element->getAttribute(key);
  if (v.empty())
    return false;

  value = v;
  return true;
}

const std::string& berry::XMLMemento::GetTextData() const
{
  Poco::XML::Text* textNode = GetTextNode();

  if (textNode != NULL)
  {
    return textNode->getData();
  }

  return EMPTY_STRING;
}

std::vector<std::string> berry::XMLMemento::GetAttributeKeys() const
{
  std::vector<std::string> values;
  Poco::XML::NamedNodeMap* nnMap = element->attributes();

  values.resize(nnMap->length());

  for (unsigned long i = 0; i < nnMap->length(); i++)
  {
    values[i] = nnMap->item(i)->nodeName(); //TODO check if right
  }

  return values;
}

Poco::XML::Text* berry::XMLMemento::GetTextNode() const
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
      return dynamic_cast<Poco::XML::Text*> (nodes->item(index));
    }
  }

  // a Text node was not found
  return NULL;

}

void berry::XMLMemento::PutElement(Poco::XML::Element* element, bool copyText)
{
  Poco::XML::NamedNodeMap* nodeMap = element->attributes();
  unsigned long size = nodeMap->length();

  for (unsigned long index = 0; index < size; index++)
  {
    Poco::XML::Node* node = nodeMap->item(index);
    Poco::XML::Attr* attr = dynamic_cast<Poco::XML::Attr*> (node);
    PutString(attr->nodeName(), attr->nodeValue());
  }
  nodeMap->release();

  bool needToCopyText = copyText;
  Poco::XML::Node* child = element->firstChild();
  while (child)
  {
    unsigned short nodeType = child->nodeType();

    switch (nodeType)
    {
    case Poco::XML::Node::ELEMENT_NODE:
    {
      Poco::XML::Element* elem = dynamic_cast<Poco::XML::Element*> (child);
      XMLMemento::Pointer child = CreateChild(elem->nodeName()).Cast<
          berry::XMLMemento> ();
      child->PutElement(elem, true);
    }
    break;
    case Poco::XML::Node::TEXT_NODE:
      if (needToCopyText)
      {
        Poco::XML::Text* text = dynamic_cast<Poco::XML::Text*> (child);
        PutTextData(text->getData());
        needToCopyText = false;
      }
      break;

    default:
      break;
    }

    child = child->nextSibling();
  }
}

void berry::XMLMemento::PutFloat(const std::string& key, double value)
{
  std::string xmlValue = Poco::NumberFormatter::format(value);
  element->setAttribute(key, xmlValue);
}

void berry::XMLMemento::PutInteger(const std::string& key, int value)
{
  std::string xmlValue = Poco::NumberFormatter::format(value);
  element->setAttribute(key, xmlValue);
}

void berry::XMLMemento::PutMemento(IMemento::Pointer memento)
{
  // Do not copy the element's top level text node (this would overwrite the existing text).
  // Text nodes of children are copied.
  PutElement(memento.Cast<berry::XMLMemento> ()->GetElement(), false);
}

void berry::XMLMemento::PutString(const std::string& key,
    const std::string& value)
{
  element->setAttribute(key, value);
  //if (value == null) {
  //  return;}
  //element.setAttribute(key, value);
}

void berry::XMLMemento::PutBoolean(const std::string& key, bool value)
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

void berry::XMLMemento::PutTextData(const std::string& data)
{
  Poco::XML::Text* textNode = GetTextNode();
  if (textNode == NULL)
  {
    textNode = factory->createTextNode(data);
    element->insertBefore(textNode, element->firstChild())->release();
  }
  else
  {
    textNode->setData(data);
  }
}

void berry::XMLMemento::Save(berry::XMLMemento::XMLByteOutputStream& writer)
{
  if (writer.good())
  {
    Poco::XML::DOMWriter out;
    out.setOptions(3); //write declaration and pretty print
    out.writeNode(writer, factory);
  }
  else
  {
    //TODO
  }
}
Poco::XML::Element* berry::XMLMemento::GetElement() const
{
  return element;
}

