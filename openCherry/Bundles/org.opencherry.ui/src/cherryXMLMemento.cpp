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

const std::string TAG_ID = "IMemento.internal.id";

cherry::XMLMemento::XMLMemento(Poco::XML::Document* document, Poco::XML::Element* elem) 
{
  factory = document;
  element = elem;
}

//static XMLMemento::Pointer XMLMemento::CreateReadRoot(Reader reader) throw(WorkbenchException)
cherry::XMLMemento::Pointer cherry::XMLMemento::CreateReadRoot(cherry::XMLMemento::XMLByteInputStream& reader) throw(WorkbenchException)
{
  return CreateReadRoot(reader, "");
  //return CreateReadRoot(reader, null);
}

cherry::XMLMemento::Pointer cherry::XMLMemento::CreateReadRoot(cherry::XMLMemento::XMLByteInputStream& reader, const std::string& baseDir) throw(WorkbenchException)
{
    //

    //try
    
      Poco::XML::NamePool* namePool = new Poco::XML::NamePool();  
      Poco::XML::Document* doc = new Poco::XML::Document(namePool);
      Poco::XML::DOMParser* parser = new Poco::XML::DOMParser();
      //Poco::XML::DOMBuilder* builder = new Poco::XML::DOMBuilder(parser,namePool);
      Poco::XML::InputSource* source = new Poco::XML::InputSource(reader);


      source->setSystemId(baseDir);
      doc = parser->parse(source);

      Poco::XML::Element* elem = doc->documentElement(); //TODO really documentElement?

      return cherry::XMLMemento::New(doc, elem);
      
      //Poco::XML::NamedNodeMap* nodes = factory->childNodes();

    
  //String errorMessage = null;
  //Exception exception = null;

  //try {
  //  DocumentBuilderFactory factory = DocumentBuilderFactory
  //    .newInstance();
  //  DocumentBuilder parser = factory.newDocumentBuilder();
  //  InputSource source = new InputSource(reader);
  //  if (baseDir != null) {
  //    source.setSystemId(baseDir);
  //  }
  //  Document document = parser.parse(source);
  //  NodeList list = document.getChildNodes();
  //  for (int i = 0; i < list.getLength(); i++) {
  //    Node node = list.item(i);
  //    if (node instanceof Element) {
  //      return new XMLMemento(document, (Element) node);
  //    }
  //  }
  //} catch (ParserConfigurationException e) {
  //  exception = e;
  //  errorMessage = WorkbenchMessages.XMLMemento_parserConfigError;
  //} catch (IOException e) {
  //  exception = e;
  //  errorMessage = WorkbenchMessages.XMLMemento_ioError;
  //} catch (SAXException e) {
  //  exception = e;
  //  errorMessage = WorkbenchMessages.XMLMemento_formatError;
  //}

  //String problemText = null;
  //if (exception != null) {
  //  problemText = exception.getMessage();
  //}
  //if (problemText == null || problemText.length() == 0) {
  //  problemText = errorMessage != null ? errorMessage
  //    : WorkbenchMessages.XMLMemento_noElement;
  //}
  //throw new WorkbenchException(problemText, exception);
}

cherry::XMLMemento::Pointer cherry::XMLMemento::CreateWriteRoot(const std::string& type) {

//  try{
    Poco::XML::NamePool* namePool = new Poco::XML::NamePool();  
    Poco::XML::Document* doc = new Poco::XML::Document(namePool);
    Poco::XML::Element* elem = doc->createElement(type);
    doc->appendChild(elem);
    

    return cherry::XMLMemento::New(doc, elem);
  //}catch() //TODO: look for poco exceptions
  //{
  //}

  //Document document;
  //try {
  //  document = DocumentBuilderFactory.newInstance()
  //    .newDocumentBuilder().newDocument();
  //  Element element = document.createElement(type);
  //  document.appendChild(element);
  //  return new XMLMemento(document, element);
  //} catch (ParserConfigurationException e) {
  //  //            throw new Error(e);
  //  throw new Error(e.getMessage());
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
  
  //Element childElement = ((XMLMemento) child).element;
  //Element newElement = (Element) factory.importNode(childElement, true);
  //element.appendChild(newElement);
  //return new XMLMemento(factory, newElement);
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
  //// Get the nodes.
  //NodeList nodes = element.getChildNodes();
  //int size = nodes.getLength();
  //if (size == 0) {
  //  return new IMemento[0];}

  //// Extract each node with given type.
  //ArrayList list = new ArrayList(size);
  //for (int nX = 0; nX < size; nX++) {
  //  Node node = nodes.item(nX);
  //  if (node instanceof Element) {
  //    Element element = (Element) node;
  //    if (element.getNodeName().equals(type)) {
  //      list.add(element);
  //    }
  //  }
  //}

  //// Create a memento for each node.
  //size = list.size();
  //IMemento[] results = new IMemento[size];
  //for (int x = 0; x < size; x++) {
  //  results[x] = new XMLMemento(factory, (Element) list.get(x));
  //}
  //return results;
}


float cherry::XMLMemento::GetFloat(const std::string& key) const
{
  //TODO: check typeconversion and make error handling!
  std::stringstream ss;
  float val=0;
  const std::string& attr = element->getAttribute(key);

 // attr >> ss >> val;

  return val;
  //Attr attr = element.getAttributeNode(key);
  //if (attr == null) {
  //  return null;}
  //String strValue = attr.getValue();
  //try {
  //  return new Float(strValue);
  //} catch (NumberFormatException e) {
  //  WorkbenchPlugin.log("Memento problem - Invalid float for key: " //$NON-NLS-1$
  //    + key + " value: " + strValue, e); //$NON-NLS-1$
  //  return null;
  //}
}

const std::string& cherry::XMLMemento::GetType() const
{
  return element->nodeName();
}

const std::string& cherry::XMLMemento::GetID() const
{

  //TODO: make error handling!
  //std::string attr = element->getAttribute(TAG_ID);

  //return attr;

  return element->getAttribute(TAG_ID);
}

int cherry::XMLMemento::GetInteger(const std::string& key) const
{
  std::stringstream ss;
  int val=0;
  std::string attr = element->getAttribute(key);

  ss << attr;
  ss >> val;

  return val;
  //Attr attr = element.getAttributeNode(key);
  //if (attr == null) {
  //  return null;}
  //String strValue = attr.getValue();
  //try {
  //  return new Integer(strValue);
  //} catch (NumberFormatException e) {
  //  WorkbenchPlugin
  //    .log("Memento problem - invalid integer for key: " + key //$NON-NLS-1$
  //    + " value: " + strValue, e); //$NON-NLS-1$
  //  return null;
  //}
}

const std::string& cherry::XMLMemento::GetString(const std::string& key) const 
{
  //std::string attr = element->getAttribute(key);
  return element->getAttribute(key);
  //Attr attr = element.getAttributeNode(key);
  //if (attr == null) {
  //  return null;}
  //return attr.getValue();
}

bool cherry::XMLMemento::GetBoolean(const std::string& key) const
{
  //TODO: what if attr contains something else then "false"?
  std::string attr = element->getAttribute(key);
  if (attr == "true")
    return true;
  else
    return false;
  //Attr attr = element.getAttributeNode(key);
  //if (attr == null) {
  //  return null;}
  //return Boolean.valueOf(attr.getValue());
}

const std::string& cherry::XMLMemento::GetTextData() const
{
  Poco::XML::Text* textNode = GetTextNode();

  if (textNode != NULL)
  {
    return textNode->getData();
  }

  return NULL;
  //Text textNode = getTextNode();
  //if (textNode != null) {
  //  return textNode.getData();
  //}
  //return null;
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
  Poco::XML::Text* text;

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

  //Poco::XML::Element* elem;
  //cherry::XMLMemento::Pointer child;

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
  //NamedNodeMap nodeMap = element.getAttributes();
  //int size = nodeMap.getLength();
  //for (int i = 0; i < size; i++) {
  //  Attr attr = (Attr) nodeMap.item(i);
  //  putString(attr.getName(), attr.getValue());
  //}

  //NodeList nodes = element.getChildNodes();
  //size = nodes.getLength();
  //// Copy first text node (fixes bug 113659).
  //// Note that text data will be added as the first child (see putTextData)
  //boolean needToCopyText = copyText;
  //for (int i = 0; i < size; i++) {
  //  Node node = nodes.item(i);
  //  if (node instanceof Element) {
  //    XMLMemento child = (XMLMemento) createChild(node.getNodeName());
  //    child.putElement((Element) node, true);
  //  } else if (node instanceof Text && needToCopyText) {
  //    putTextData(((Text) node).getData());
  //    needToCopyText = false;
  //  }
  //}
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
  //element.setAttribute(key, value ? "true" : "false"); //$NON-NLS-1$ //$NON-NLS-2$
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
  //Text textNode = getTextNode();
  //if (textNode == null) {
  //  textNode = factory.createTextNode(data);
  //  // Always add the text node as the first child (fixes bug 93718)
  //  element.insertBefore(textNode, element.getFirstChild());
  //} else {
  //  textNode.setData(data);
  //}
}

void cherry::XMLMemento::Save(cherry::XMLMemento::XMLByteOutputStream& writer)
{
  if (writer.good())
  {
    Poco::XML::DOMWriter* out = new Poco::XML::DOMWriter();
    out->setOptions(3); //write declaration
    //out->setOptions(2); //make pretty print
    out->writeNode(writer, factory);
  }
  else
  {
    //TODO
  }
  //DOMWriter out = new DOMWriter(writer);
  //try {
  //  out.print(element);
  //} finally {
  //  out.close();
  //}
}
Poco::XML::Element* cherry::XMLMemento::GetElement() const
{
  return element;
}

