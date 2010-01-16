#include "berryXMLPreferencesStorage.h"
#include "berryPreferences.h"

#include "berryLog.h"
#define WARNMSG BERRY_WARN("XMLPreferencesStorage")

#include "Poco/File.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/DOM/Element.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/SAX/InputSource.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/Element.h"
#include "Poco/Exception.h"
#include "Poco/SAX/SAXException.h"

using Poco::XML::DOMParser;
using Poco::XML::InputSource;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::Element;
using Poco::XML::AutoPtr;
using Poco::Exception;

#include <iostream>
#include <fstream>

namespace berry 
{


  XMLPreferencesStorage::XMLPreferencesStorage( const Poco::File& _File )
    : AbstractPreferencesStorage(_File)
  {
    // file already exists
    try{
      if(_File.exists())
      {
        // build preferences tree from file
        InputSource src(_File.path());
        DOMParser parser;
        AutoPtr<Document> pDoc = parser.parse(&src);
        this->ToPreferencesTree(pDoc->documentElement(), 0);
      }
    }
    catch (Poco::XML::SAXParseException& exc)
    {
      const std::string tempString = _File.path()+".backup";
      BERRY_INFO << exc.what();
      BERRY_INFO << "Preferences could not be loaded."; 
      BERRY_INFO << "Creating " << tempString; 
      BERRY_INFO << "and resetting to default values.";
      _File.copyTo(tempString);
      this->m_Root = NULL;
    }

    // if root is null make new one
    if(this->m_Root.IsNull())
    {
      this->m_Root = Preferences::Pointer(new Preferences(Preferences::PropertyMap(), "", 0, this));
    }
  }



  void XMLPreferencesStorage::Flush(IPreferences*  /*_Preferences*/) throw(Poco::Exception, BackingStoreException)
  {
    try
    {
	    this->ToDOMTree(dynamic_cast<Preferences*>(this->m_Root.GetPointer()), 0);
	    // flush dom to file	
	    Poco::XML::DOMWriter writer;
	    //writer.setNewLine("\n");
      writer.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);
      std::ofstream f;
      f.open (this->m_File.path().c_str());
      
      // PRECISION setting
      // f.precision(10);
      writer.writeNode(f, m_Document);

      f.close();
    }
    catch (std::exception& e)
    {
      WARNMSG << e.what();
    }
  }

  XMLPreferencesStorage::~XMLPreferencesStorage()
  {

  }

  void XMLPreferencesStorage::ToPreferencesTree( Poco::XML::Node* DOMNode, Preferences* prefParentNode )
  {
    // collect properties from attributes
    Preferences::PropertyMap properties;
    Poco::XML::Element* elem = dynamic_cast<Poco::XML::Element*>(DOMNode);
    std::string name = elem->getAttribute("name");

    // create a new Preferences node
    Preferences::Pointer newNode(new Preferences(properties, name, prefParentNode, this));
    // save the new Preferences node as root node if needed
    if(prefParentNode == 0)
      m_Root = newNode;

    // recursion: now go on for all child nodes of DOMNode
    Poco::XML::NodeList* childNodes = elem->childNodes();
    Poco::XML::Node* currentNode = 0;
    Poco::XML::Element* prop = 0;
    std::string key; 
    std::string value;

    for (unsigned int i = 0; i != childNodes->length(); ++i)
    {
      currentNode = childNodes->item(i);

      if(currentNode->nodeName() == "preferences")
        this->ToPreferencesTree(currentNode, newNode.GetPointer());
      else if(currentNode->nodeName() == "property")
      {
        prop = dynamic_cast<Poco::XML::Element*>(currentNode);
        key = prop->getAttribute("name");
        value = prop->getAttribute("value");
        newNode->Put(key, value);
      }
    }
  }

  void XMLPreferencesStorage::ToDOMTree( Preferences* prefNode, Poco::XML::Node* parentDOMNode )
  {
    if(parentDOMNode == 0)
    {
      m_Document = new Poco::XML::Document;
      parentDOMNode = m_Document;
    }

    //# create DOMNode from Preferences    
    Element* newNode = m_Document->createElement("preferences");
    newNode->setAttribute("name", prefNode->Name());

    // make attributes
    Preferences::PropertyMap properties = prefNode->GetProperties();
    Element* newProp = 0;
    for (Preferences::PropertyMap::iterator it = properties.begin()
      ; it != properties.end(); it++)
    {
      Element* newProp = m_Document->createElement("property");
      newProp->setAttribute("name", it->first);
      newProp->setAttribute("value", it->second);
      newNode->appendChild(newProp);
    }
    // save in parentDOMNode
    parentDOMNode->appendChild(newNode);

    // recursion: now do that for all child Preferences nodes
    Preferences::ChildrenList children = prefNode->GetChildren();
    for (Preferences::ChildrenList::iterator it = children.begin()
      ; it != children.end(); it++)
    {
      this->ToDOMTree((*it).GetPointer(), newNode);
    }

  }
}
