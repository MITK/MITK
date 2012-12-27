/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include <mitkXMLSerializable.h>
#include "mitkEndoDebug.h"
#include "mitkEndoMacros.h"
#include <itksys/SystemTools.hxx>

namespace mitk
{
  const std::string XMLSerializable::FILE_REFERENCE_ATTRIBUTE_NAME = "fileRef";
  const std::string XMLSerializable::ROOT_NAME = "data";

  void mitk::XMLSerializable::ToXMLFile(const std::string& file
                                        , const std::string& elemName)
  {
    TiXmlElement * rootElem=0;
    TiXmlElement * element=0;

    // determine element to write to
    std::string elementName = elemName;
    if(elementName.empty())
      elementName = this->GetNameOfClass();

    TiXmlDocument doc( file.c_str() );
    bool loadOkay = doc.LoadFile();
    // if the document already exists ...
    if(loadOkay)
    {
      // try to identify the XML element of this class as the root
      // or as the child of the root
      rootElem = doc.RootElement();
      endoAssertMsg( rootElem, "No root element found in " << file );

      // if root element represents this element remove the root
      if( rootElem->ValueStr() == elementName )
      {
        doc.RemoveChild(rootElem);
        rootElem = 0;
      }
      else
      {
        // if it is a child of the root remove it too
        element = rootElem->FirstChildElement(elementName);
        if(element)
          rootElem->RemoveChild(element);
      }
    }
    else
    {
      // document did not exist, create new one with declration
      TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
      doc.LinkEndChild( decl );
    }

    m_XMLFileName = file;

    // create element (if the document already exists this element was removed)
    element = new TiXmlElement( elementName );
    this->ToXML( element );

    // if we do not have a root element create a new one
    if(!rootElem)
      rootElem = new TiXmlElement( ROOT_NAME );
    // add the element node as child
    rootElem->LinkEndChild(element);

    // if no root element exists, add it now
    if(doc.RootElement() == 0)
      doc.LinkEndChild( rootElem );

    if(!doc.SaveFile( file ))
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

    TiXmlDocument doc( file.c_str() );
    bool loadOkay = doc.LoadFile();
    if(!loadOkay)
    {
      std::ostringstream s; s << "File " << file
          << " could not be loaded!";
      throw std::logic_error(s.str().c_str());
    }

    m_XMLFileName = file;

    TiXmlElement* elem = doc.FirstChildElement();
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
    std::string filename;
    if(elem->QueryStringAttribute(FILE_REFERENCE_ATTRIBUTE_NAME.c_str(), &filename)
      == TIXML_SUCCESS)
    {
      if( !itksys::SystemTools::FileIsFullPath(filename.c_str()) )
        filename = itksys::SystemTools::GetFilenamePath(file) + "/" + filename;
      this->FromXMLFile(filename);
      return; // exit!
    }

    this->FromXML( elem );
  }
}
