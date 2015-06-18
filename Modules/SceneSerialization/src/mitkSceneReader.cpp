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

#include "mitkSceneReader.h"

bool mitk::SceneReader::LoadScene( TiXmlDocument& document, const std::string& workingDirectory, DataStorage* storage )
{
  // find version node --> note version in some variable
  int fileVersion = 1;
  TiXmlElement* versionObject = document.FirstChildElement("Version");
  if (versionObject)
  {
    if ( versionObject->QueryIntAttribute( "FileVersion", &fileVersion ) != TIXML_SUCCESS )
    {
      MITK_ERROR << "Scene file " << workingDirectory + "/index.xml" << " does not contain version information! Trying version 1 format." << std::endl;
    }
  }

  std::stringstream sceneReaderClassName;
  sceneReaderClassName << "SceneReaderV" << fileVersion;

  std::list<itk::LightObject::Pointer> sceneReaders = itk::ObjectFactoryBase::CreateAllInstance(sceneReaderClassName.str().c_str());
  if (sceneReaders.size() < 1)
  {
    MITK_ERROR << "No scene reader found for scene file version " << fileVersion;
  }
  if (sceneReaders.size() > 1)
  {
    MITK_WARN << "Multiple scene readers found for scene file version " << fileVersion << ". Using arbitrary first one.";
  }

  for ( auto iter = sceneReaders.begin();
        iter != sceneReaders.end();
        ++iter )
  {
    if (SceneReader* reader = dynamic_cast<SceneReader*>( iter->GetPointer() ) )
    {
      if ( !reader->LoadScene( document, workingDirectory, storage ) )
      {
        MITK_ERROR << "There were errors while loading scene file " << workingDirectory + "/index.xml. Your data may be corrupted";
        return false;
      }
      else
      {
        return true;
      }
    }
  }
  return false;
}
