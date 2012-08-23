#include "mitkEndoDebugFromXmlFile.h"
#include <itksys/SystemTools.hxx>
#include <tinyxml.h>
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
      int found;
      found = str.find_first_of(separator);
      while(found != string::npos){
          if(found > 0){
              results->insert(str.substr(0,found));
          }
          str = str.substr(found+1);
          found = str.find_first_of(separator);
      }
      if(str.length() > 0){
          results->insert(str);
      }
  }

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
    endodebugvar( _FileName )
    TiXmlDocument doc( _FileName );
    doc.LoadFile();
    TiXmlHandle docHandle( &doc );
    TiXmlElement* elem = docHandle.FirstChildElement().FirstChildElement( "EndoDebug" ).ToElement();

    if(elem == 0)
    {
      endodebug("EndoDebug element not found");
      return;
    }

    int _DebugEnabled = d->m_EndoDebug->GetDebugEnabled();
    if( elem->QueryIntAttribute("DebugEnabled",&_DebugEnabled) != TIXML_SUCCESS )
      endodebug("DebugEnabled attribute not found");

    int _ShowImagesInDebug = d->m_EndoDebug->GetShowImagesInDebug();
    if( elem->QueryIntAttribute("ShowImagesInDebug",&_ShowImagesInDebug) != TIXML_SUCCESS )
      endodebug("ShowImagesInDebug attribute not found");

    int _ShowImagesTimeOut = d->m_EndoDebug->GetShowImagesTimeOut();
    if( elem->QueryIntAttribute("ShowImagesTimeOut",&_ShowImagesTimeOut) != TIXML_SUCCESS )
      endodebug("ShowImagesTimeOut attribute not found");

    std::string _DebugImagesOutputDirectory = d->m_EndoDebug->GetDebugImagesOutputDirectory();
    if( elem->QueryStringAttribute("DebugImagesOutputDirectory",&_DebugImagesOutputDirectory) != TIXML_SUCCESS )
      endodebug("DebugImagesOutputDirectory attribute not found");

    std::set<std::string> _FilesToDebug;
    std::string _FilesToDebugString;
    if( elem->QueryStringAttribute("FilesToDebug",&_FilesToDebugString) != TIXML_SUCCESS )
    {
      endodebug("FilesToDebug attribute not found");
    }
    else
    {
        StringExplode( _FilesToDebugString, ";", &_FilesToDebug );
    }

    std::set<std::string> _SymbolsToDebug;
    std::string _SymbolsToDebugString;
    if( elem->QueryStringAttribute("SymbolsToDebug",&_SymbolsToDebugString) != TIXML_SUCCESS )
    {
      endodebug("SymbolsToDebug attribute not found");
    }
    else
    {
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

}
