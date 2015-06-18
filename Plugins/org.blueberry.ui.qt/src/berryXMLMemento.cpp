/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

#include <limits>

const QString EMPTY_STRING;

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
    berry::XMLMemento::XMLByteInputStream& reader)
{
  return CreateReadRoot(reader, "");
}

berry::XMLMemento::Pointer berry::XMLMemento::CreateReadRoot(
    berry::XMLMemento::XMLByteInputStream& reader, const QString& baseDir)
{
  QString errorMessage;
  Poco::Exception exception("");

  try
  {
    Poco::XML::DOMParser parser;
    Poco::XML::InputSource source(reader);

    source.setSystemId(baseDir.toStdString());
    Poco::XML::Document* doc = parser.parse(&source);

    Poco::XML::Element* elem = doc->documentElement();

    XMLMemento::Pointer memento(new XMLMemento(doc, elem));

    doc->release();

    return memento;
  }
  catch (Poco::XML::SAXParseException& e)
  {
    errorMessage = QString("Could not parse content of XML file: ") + QString::fromStdString(e.displayText());
  }

  QString problemText = QString::fromStdString(exception.message());
  if (problemText.isEmpty())
  {
    problemText = errorMessage.isEmpty() ? "Could not find root element node of XML file." : errorMessage;
  }
  throw WorkbenchException(problemText);

}

berry::XMLMemento::Pointer berry::XMLMemento::CreateWriteRoot(
    const QString& type)
{
  // TODO
  //  try{
  auto   doc = new Poco::XML::Document();
  Poco::XML::Element* elem = doc->createElement(type.toStdString());
  doc->appendChild(elem)->release();

  XMLMemento::Pointer memento(new XMLMemento(doc, elem));
  doc->release();
  return memento;
  //}catch() //TODO: look for poco exceptions
  //{
  //}
}

berry::IMemento::Pointer berry::XMLMemento::CreateChild(
    const QString& type)
{
  Poco::XML::Element* child = factory->createElement(type.toStdString());
  element->appendChild(child)->release();
  IMemento::Pointer xmlChild(new XMLMemento(factory,child));
  return xmlChild;
}

berry::IMemento::Pointer berry::XMLMemento::CreateChild(
    const QString& type, const QString& id)
{
  Poco::XML::Element* child = factory->createElement(type.toStdString());
  child->setAttribute(TAG_ID.toStdString(), id.toStdString());
  element->appendChild(child)->release();
  IMemento::Pointer xmlChild(new XMLMemento(factory,child));
  return xmlChild;
}

berry::IMemento::Pointer berry::XMLMemento::CopyChild(IMemento::Pointer child)
{
  //TODO check any casting errors
  Poco::XML::Element* elem = child.Cast<berry::XMLMemento> ()->GetElement();
  Poco::XML::Element* newElement =
      dynamic_cast<Poco::XML::Element*> (factory->importNode(elem, true));
  element->appendChild(newElement)->release();
  IMemento::Pointer xmlCopy(new XMLMemento(factory, newElement));
  return xmlCopy;

}

berry::IMemento::Pointer berry::XMLMemento::GetChild(const QString& type) const
{
  // Get the nodes.
  berry::XMLMemento::Pointer memento;
  Poco::XML::Element* child = element->getChildElement(type.toStdString()); // Find the first node which is a child of this node
  if (child)
  {
    memento = new berry::XMLMemento(factory, child);
    return memento;
  }
  // A child was not found.
  return memento;
}

QList<berry::IMemento::Pointer> berry::XMLMemento::GetChildren(
    const QString& type) const
{
  QList<IMemento::Pointer> mementos;
  Poco::XML::NodeList* elementList = element->getElementsByTagName(type.toStdString());
  for (unsigned long i = 0; i < elementList->length(); i++)
  {
    Poco::XML::Element* elem =
        dynamic_cast<Poco::XML::Element*> (elementList->item(i));
    IMemento::Pointer child(new berry::XMLMemento(factory, elem));
    mementos << child;
  }
  elementList->release();

  return mementos;
}

bool berry::XMLMemento::GetFloat(const QString& key, double& value) const
{
  if (!element->hasAttribute(key.toStdString())) return false;

  const std::string& attr = element->getAttribute(key.toStdString());

  try
  {
    value = Poco::NumberParser::parseFloat(attr);
  } catch (const Poco::SyntaxException& /*e*/)
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
        + " value: " + QString::fromStdString(attr));
    return false;
  }

  return true;
}

QString berry::XMLMemento::GetType() const
{
  return QString::fromStdString(element->nodeName());
}

QString berry::XMLMemento::GetID() const
{
  //TODO: make error handling!
  return QString::fromStdString(element->getAttribute(TAG_ID.toStdString()));
}

bool berry::XMLMemento::GetInteger(const QString& key, int& value) const
{
  if (!element->hasAttribute(key.toStdString())) return false;

  const std::string& attr = element->getAttribute(key.toStdString());

  try
  {
    value = Poco::NumberParser::parse(attr);
  }
  catch (const Poco::SyntaxException& /*e*/)
  {
    WorkbenchPlugin::Log("Memento problem - invalid integer for key: " + key
                         + " value: " + QString::fromStdString(attr));
    return false;
  }

  return true;
}

bool berry::XMLMemento::GetBoolean(const QString& key, bool& value) const
{
  const std::string& attr = element->getAttribute(key.toStdString());
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

bool berry::XMLMemento::GetString(const QString& key, QString& value) const
{
  QString v = QString::fromStdString(element->getAttribute(key.toStdString()));
  if (v.isEmpty())
    return false;

  value = v;
  return true;
}

QString berry::XMLMemento::GetTextData() const
{
  Poco::XML::Text* textNode = GetTextNode();

  if (textNode != nullptr)
  {
    return QString::fromStdString(textNode->getData());
  }

  return EMPTY_STRING;
}

QList<QString> berry::XMLMemento::GetAttributeKeys() const
{
  QList<QString> values;
  Poco::XML::NamedNodeMap* nnMap = element->attributes();

  values.reserve(nnMap->length());

  for (unsigned long i = 0; i < nnMap->length(); i++)
  {
    values[i] = QString::fromStdString(nnMap->item(i)->nodeName()); //TODO check if right
  }

  return values;
}

Poco::XML::Text* berry::XMLMemento::GetTextNode() const
{

  //Get the nodes
  Poco::XML::NodeList* nodes = element->childNodes();

  unsigned long size = nodes->length();

  if (size == 0)
    return nullptr;

  //Search for the text node
  for (unsigned long index = 0; index < size; index++)
  {
    if (nodes->item(index)->nodeType() == Poco::XML::Node::TEXT_NODE)
    {
      return dynamic_cast<Poco::XML::Text*> (nodes->item(index));
    }
  }

  // a Text node was not found
  return nullptr;

}

void berry::XMLMemento::PutElement(Poco::XML::Element* element, bool copyText)
{
  Poco::XML::NamedNodeMap* nodeMap = element->attributes();
  unsigned long size = nodeMap->length();

  for (unsigned long index = 0; index < size; index++)
  {
    Poco::XML::Node* node = nodeMap->item(index);
    Poco::XML::Attr* attr = dynamic_cast<Poco::XML::Attr*> (node);
    PutString(QString::fromStdString(attr->nodeName()),
              QString::fromStdString(attr->nodeValue()));
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
      XMLMemento::Pointer child = CreateChild(QString::fromStdString(elem->nodeName())).Cast<
          berry::XMLMemento> ();
      child->PutElement(elem, true);
    }
    break;
    case Poco::XML::Node::TEXT_NODE:
      if (needToCopyText)
      {
        Poco::XML::Text* text = dynamic_cast<Poco::XML::Text*> (child);
        PutTextData(QString::fromStdString(text->getData()));
        needToCopyText = false;
      }
      break;

    default:
      break;
    }

    child = child->nextSibling();
  }
}

void berry::XMLMemento::PutFloat(const QString& key, double value)
{
  std::string xmlValue = Poco::NumberFormatter::format(value);
  element->setAttribute(key.toStdString(), xmlValue);
}

void berry::XMLMemento::PutInteger(const QString& key, int value)
{
  std::string xmlValue = Poco::NumberFormatter::format(value);
  element->setAttribute(key.toStdString(), xmlValue);
}

void berry::XMLMemento::PutMemento(IMemento::Pointer memento)
{
  // Do not copy the element's top level text node (this would overwrite the existing text).
  // Text nodes of children are copied.
  PutElement(memento.Cast<berry::XMLMemento> ()->GetElement(), false);
}

void berry::XMLMemento::PutString(const QString& key,
    const QString& value)
{
  element->setAttribute(key.toStdString(), value.toStdString());
  //if (value == null) {
  //  return;}
  //element.setAttribute(key, value);
}

void berry::XMLMemento::PutBoolean(const QString& key, bool value)
{
  if (value)
  {
    element->setAttribute(key.toStdString(), "true");
  }
  else
  {
    element->setAttribute(key.toStdString(), "false");
  }
}

void berry::XMLMemento::PutTextData(const QString& data)
{
  Poco::XML::Text* textNode = GetTextNode();
  if (textNode == nullptr)
  {
    textNode = factory->createTextNode(data.toStdString());
    element->insertBefore(textNode, element->firstChild())->release();
  }
  else
  {
    textNode->setData(data.toStdString());
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

