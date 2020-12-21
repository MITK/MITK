/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkEndoDebugFromXmlFile.h"
#include <itksys/SystemTools.hxx>
#include <tinyxml2.h>
#include <set>
using namespace std;

namespace mitk
{
  struct EndoDebugFromXmlFileData
  {
    const std::string* m_FileName;

    // private
    EndoDebug* m_EndoDebug;
    long int m_FileModifiedTime;
  };

  EndoDebugFromXmlFile::EndoDebugFromXmlFile(
      const std::string* _FileName, EndoDebug* _EndoDebug )
    : d( new EndoDebugFromXmlFileData )
  {
      d->m_FileName = _FileName;
      d->m_EndoDebug = _EndoDebug;
      d->m_FileModifiedTime = 0;
  }

  EndoDebugFromXmlFile::~EndoDebugFromXmlFile()
  {
    delete d;
  }

  void StringExplode(string str, string separator, set<string>* results){
      std::size_t found;
      found = str.find_first_of(separator);
      while(found != string::npos){
          if(found != 0){
              results->insert(str.substr(0,found));
          }
          str = str.substr(found+1);
          found = str.find_first_of(separator);
      }
      if(!str.empty()){
          results->insert(str);
      }
  }

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable:4390)
#endif

  void EndoDebugFromXmlFile::Update()
  {
    endodebug( __FUNCTION__ )

    std::string _FileName = *d->m_FileName;
    if( !itksys::SystemTools::FileExists( _FileName.c_str() ) )
    {
      endodebug(_FileName << " does not exist");
      return;
    }

    long int _FileModifiedTime
      = itksys::SystemTools::ModifiedTime( _FileName.c_str() );
    // file has changed: we know an older version...
    if( d->m_FileModifiedTime >= _FileModifiedTime )
    {
        endodebug("File not changed. No Update necessary.");
        return;
    }

    // reread
    endodebugvar(_FileName)
    tinyxml2::XMLDocument doc;
    doc.LoadFile(_FileName.c_str());
    tinyxml2::XMLHandle docHandle(&doc);
    auto* elem = docHandle.FirstChildElement().FirstChildElement( "EndoDebug" ).ToElement();

    if(elem == nullptr)
    {
      endodebug("EndoDebug element not found");
      return;
    }

    int _DebugEnabled = d->m_EndoDebug->GetDebugEnabled();
    if( elem->QueryIntAttribute("DebugEnabled",&_DebugEnabled) != tinyxml2::XML_SUCCESS )
    {
      endodebug("DebugEnabled attribute not found");
    }

    int _ShowImagesInDebug = d->m_EndoDebug->GetShowImagesInDebug();
    if( elem->QueryIntAttribute("ShowImagesInDebug",&_ShowImagesInDebug) != tinyxml2::XML_SUCCESS)
    {
      endodebug("ShowImagesInDebug attribute not found");
    }

    int _ShowImagesTimeOut = static_cast<int>(d->m_EndoDebug->GetShowImagesTimeOut());
    if( elem->QueryIntAttribute("ShowImagesTimeOut",&_ShowImagesTimeOut) != tinyxml2::XML_SUCCESS)
    {
      endodebug("ShowImagesTimeOut attribute not found");
    }

    std::string _DebugImagesOutputDirectory;
    const char* _DebugImagesOutputDirectoryC = elem->Attribute("DebugImagesOutputDirectory");
    if(nullptr == _DebugImagesOutputDirectoryC)
    {
      _DebugImagesOutputDirectory = d->m_EndoDebug->GetDebugImagesOutputDirectory();
      endodebug("DebugImagesOutputDirectory attribute not found");
    }
    else
    {
      _DebugImagesOutputDirectory = _DebugImagesOutputDirectoryC;
    }

    std::set<std::string> _FilesToDebug;
    std::string _FilesToDebugString;
    const char* _FilesToDebugStringC = elem->Attribute("FilesToDebug");
    if(nullptr == _FilesToDebugStringC)
    {
      endodebug("FilesToDebug attribute not found");
    }
    else
    {
      _FilesToDebugString = _FilesToDebugStringC;
      StringExplode( _FilesToDebugString, ";", &_FilesToDebug );
    }

    std::set<std::string> _SymbolsToDebug;
    std::string _SymbolsToDebugString;
    const char* _SymbolsToDebugStringC = elem->Attribute("SymbolsToDebug");
    if( nullptr == _SymbolsToDebugStringC )
    {
      endodebug("SymbolsToDebug attribute not found");
    }
    else
    {
      _SymbolsToDebugString = _SymbolsToDebugStringC;
      StringExplode( _SymbolsToDebugString, ";", &_SymbolsToDebug );
    }

    // save
    mitk::EndoDebug::GetInstance().SetDebugEnabled( _DebugEnabled == 1? true: false );
    mitk::EndoDebug::GetInstance().SetShowImagesInDebug( _ShowImagesInDebug == 1? true: false  );
    mitk::EndoDebug::GetInstance().SetShowImagesTimeOut( _ShowImagesTimeOut );
    mitk::EndoDebug::GetInstance().SetDebugImagesOutputDirectory( _DebugImagesOutputDirectory );
    mitk::EndoDebug::GetInstance().SetFilesToDebug(_FilesToDebug);
    mitk::EndoDebug::GetInstance().SetSymbolsToDebug(_SymbolsToDebug);

    // save that modified time
    d->m_FileModifiedTime = _FileModifiedTime;
  }

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

}
