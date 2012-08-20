
#include "mitkEndoDebugToXmlFile.h"
#include <tinyxml.h>

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

    TiXmlDocument doc( _FileName.c_str() );
    TiXmlElement* root = 0;
    TiXmlElement* elem = 0;
    // check if element is already available
    if(doc.LoadFile())
    {
      root = doc.FirstChildElement("data");
      if(root)
      {
        elem = root->FirstChildElement( "EndoDebug" );
        if(elem)
          root->RemoveChild(elem);
        elem = 0;
      }
    }
    else
    {
      // document did not exist, create new one with declration
      TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
      doc.LinkEndChild( decl );
      // create root
      root = new TiXmlElement( "data" );
      doc.LinkEndChild( root );
    }

    // create elem if not existent
    elem = new TiXmlElement( "EndoDebug" );

    elem->SetAttribute( "DebugEnabled",
                        (d->m_EndoDebug->GetDebugEnabled()? 1:0) );
    elem->SetAttribute( "ShowImagesInDebug",
                        (d->m_EndoDebug->GetShowImagesInDebug()? 1:0) );
    elem->SetAttribute( "ShowImagesTimeOut",
                        d->m_EndoDebug->GetShowImagesTimeOut() );
    elem->SetAttribute( "DebugImagesOutputDirectory",
                        d->m_EndoDebug->GetDebugImagesOutputDirectory() );

    std::set<std::string> _FilesToDebug = d->m_EndoDebug->GetFilesToDebug();
    std::string _FilesToDebugString;
    std::set<std::string>::iterator it = _FilesToDebug.begin();
    while( it != _FilesToDebug.end() )
    {
        if( it != _FilesToDebug.begin() )
            _FilesToDebugString.append( ";" );
        _FilesToDebugString.append( *it );
        ++it;
    }
    elem->SetAttribute( "FilesToDebug", _FilesToDebugString );

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
    elem->SetAttribute( "SymbolsToDebug", _SymbolsToDebugString );

    endodebug("adding the EndoDebug as child element of the data node")
    root->LinkEndChild(elem);

    endodebug("saving file " << _FileName)
    if( !doc.SaveFile( _FileName ) )
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
