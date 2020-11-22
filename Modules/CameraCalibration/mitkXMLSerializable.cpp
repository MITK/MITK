/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkXMLSerializable.h>
#include "mitkEndoDebug.h"
#include "mitkEndoMacros.h"
#include <itksys/SystemTools.hxx>
#include <tinyxml2.h>

namespace mitk
{
  const std::string XMLSerializable::FILE_REFERENCE_ATTRIBUTE_NAME = "fileRef";
  const std::string XMLSerializable::ROOT_NAME = "data";

  void mitk::XMLSerializable::ToXMLFile(const std::string& file
                                        , const std::string& elemName)
  {
    tinyxml2::XMLElement * rootElem=nullptr;
    tinyxml2::XMLElement* element=nullptr;

    // determine element to write to
    std::string elementName = elemName;
    if(elementName.empty())
      elementName = this->GetNameOfClass();

    tinyxml2::XMLDocument doc;
    // if the document already exists ...
    if(tinyxml2::XML_SUCCESS == doc.LoadFile(file.c_str()))
    {
      // try to identify the XML element of this class as the root
      // or as the child of the root
      rootElem = doc.RootElement();
      endoAssertMsg( rootElem, "No root element found in " << file );

      // if root element represents this element remove the root
      if( std::string(rootElem->Value() != nullptr ? rootElem->Value() : "") == elementName )
      {
        doc.DeleteChild(rootElem);
        rootElem = nullptr;
      }
      else
      {
        // if it is a child of the root remove it too
        element = rootElem->FirstChildElement(elementName.c_str());
        if(element)
          rootElem->DeleteChild(element);
      }
    }
    else
    {
      // document did not exist, create new one with declration
      doc.InsertEndChild( doc.NewDeclaration() );
    }

    m_XMLFileName = file;

    // create element (if the document already exists this element was removed)
    element = doc.NewElement( elementName.c_str() );
    this->ToXML( element );

    // if we do not have a root element create a new one
    if(!rootElem)
      rootElem = doc.NewElement( ROOT_NAME.c_str() );
    // add the element node as child
    rootElem->InsertEndChild(element);

    // if no root element exists, add it now
    if(doc.RootElement() == nullptr)
      doc.InsertEndChild( rootElem );

    if(tinyxml2::XML_SUCCESS != doc.SaveFile( file.c_str() ))
    {
      std::ostringstream s; s << "File " << file
          << " could not be written. Please check permissions.";
      throw std::logic_error(s.str());
    }
  }

  std::string mitk::XMLSerializable::GetXMLFileName() const
  {
    return m_XMLFileName;
  }

  void mitk::XMLSerializable::FromXMLFile(const std::string& file
                                          , const std::string& elemName)
  {
    endodebug( "Trying to read from " << file )

    tinyxml2::XMLDocument doc;
    if(tinyxml2::XML_SUCCESS != doc.LoadFile(file.c_str()))
    {
      std::ostringstream s; s << "File " << file
          << " could not be loaded!";
      throw std::logic_error(s.str().c_str());
    }

    m_XMLFileName = file;

    auto* elem = doc.FirstChildElement();
    endoAssertMsg( elem, "No root element found" );

    // determine element to read from
    std::string elementName = elemName;
    if(elementName.empty())
      elementName = this->GetNameOfClass();
    // try again with the first element
    if(strcmp(elem->Value(), elementName.c_str()) != 0)
      elem = elem->FirstChildElement(elementName.c_str());

    endoAssertMsg( elem, "No child element \"" << elementName <<
                 "\" found in " << file );

    // if theres an attribute as file reference try to load the class
    // from that file
    const char* filenameC = elem->Attribute(FILE_REFERENCE_ATTRIBUTE_NAME.c_str());
    std::string filename = nullptr != filenameC
      ? filenameC
      : "";
    if(!filename.empty())
    {
      if( !itksys::SystemTools::FileIsFullPath(filename.c_str()) )
        filename = itksys::SystemTools::GetFilenamePath(file) + "/" + filename;
      this->FromXMLFile(filename);
      return; // exit!
    }

    this->FromXML( elem );
  }
}
