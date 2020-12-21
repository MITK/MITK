/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkEndoDebugToXmlFile.h"
#include <tinyxml2.h>

namespace mitk
{
  struct EndoDebugToXmlFileData
  {
    EndoDebug* m_EndoDebug;
    const std::string* m_FileName;
  };

  EndoDebugToXmlFile::EndoDebugToXmlFile(EndoDebug *_EndoDebug,
    const std::string* _FileName)
    : d( new EndoDebugToXmlFileData )
  {
    d->m_EndoDebug = _EndoDebug;
    d->m_FileName = _FileName;
  }

  EndoDebugToXmlFile::~EndoDebugToXmlFile()
  {
    delete d;
  }

  void EndoDebugToXmlFile::Update()
  {
    std::string _FileName = *d->m_FileName;

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* root = nullptr;
    tinyxml2::XMLElement* elem = nullptr;
    // check if element is already available
    if(tinyxml2::XML_SUCCESS == doc.LoadFile(_FileName.c_str()))
    {
      root = doc.FirstChildElement("data");
      if(nullptr != root)
      {
        elem = root->FirstChildElement( "EndoDebug" );
        if (nullptr != elem)
        {
          root->DeleteChild(elem);
          elem = nullptr;
        }
      }
    }
    else
    {
      // document did not exist, create new one with declration
      doc.InsertEndChild( doc.NewDeclaration() );
      // create root
      root = doc.NewElement( "data" );
      doc.InsertEndChild( root );
    }

    // create elem if not existent
    elem = doc.NewElement( "EndoDebug" );

    elem->SetAttribute( "DebugEnabled",
                        (d->m_EndoDebug->GetDebugEnabled()? 1:0) );
    elem->SetAttribute( "ShowImagesInDebug",
                        (d->m_EndoDebug->GetShowImagesInDebug()? 1:0) );
    elem->SetAttribute( "ShowImagesTimeOut",
                        (static_cast<int>(d->m_EndoDebug->GetShowImagesTimeOut())) );
    elem->SetAttribute( "DebugImagesOutputDirectory",
                        d->m_EndoDebug->GetDebugImagesOutputDirectory().c_str() );

    std::set<std::string> _FilesToDebug = d->m_EndoDebug->GetFilesToDebug();
    std::string _FilesToDebugString;
    auto it = _FilesToDebug.begin();
    while( it != _FilesToDebug.end() )
    {
        if( it != _FilesToDebug.begin() )
            _FilesToDebugString.append( ";" );
        _FilesToDebugString.append( *it );
        ++it;
    }
    elem->SetAttribute( "FilesToDebug", _FilesToDebugString.c_str() );

    std::set<std::string> _SymbolsToDebug = d->m_EndoDebug->GetSymbolsToDebug();
    std::string _SymbolsToDebugString;
    it = _SymbolsToDebug.begin();
    while( it != _SymbolsToDebug.end() )
    {
        if( it != _SymbolsToDebug.begin() )
            _SymbolsToDebugString.append( ";" );
        _SymbolsToDebugString.append( *it );
        ++it;
    }
    elem->SetAttribute( "SymbolsToDebug", _SymbolsToDebugString.c_str() );

    endodebug("adding the EndoDebug as child element of the data node")
    root->InsertEndChild(elem);

    endodebug("saving file " << _FileName)
    if( tinyxml2::XML_SUCCESS != doc.SaveFile( _FileName.c_str() ) )
    {
        endodebug("File " << _FileName << " could not be written. Please check permissions.");
    }
  }

  void EndoDebugToXmlFile::SetEndoDebug(EndoDebug *_EndoDebug)
  {
      d->m_EndoDebug = _EndoDebug;
  }

  void EndoDebugToXmlFile::SetFileName(const std::string *_FileName)
  {
      d->m_FileName = _FileName;
  }
}
