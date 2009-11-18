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
      LOG_ERROR << "Scene file " << workingDirectory + "/index.xml" << " does not contain version information! Trying version 1 format." << std::endl;
    }
  }
  
  std::stringstream sceneReaderClassName;
  sceneReaderClassName << "SceneReaderV" << fileVersion;

  std::list<itk::LightObject::Pointer> sceneReaders = itk::ObjectFactoryBase::CreateAllInstance(sceneReaderClassName.str().c_str());
  if (sceneReaders.size() < 1)
  {
    LOG_ERROR << "No scene reader found for scene file version " << fileVersion;
  }
  if (sceneReaders.size() > 1)
  {
    LOG_WARN << "Multiple scene readers found for scene file version " << fileVersion << ". Using arbitrary first one.";
  }

  for ( std::list<itk::LightObject::Pointer>::iterator iter = sceneReaders.begin();
        iter != sceneReaders.end();
        ++iter )
  {
    if (SceneReader* reader = dynamic_cast<SceneReader*>( iter->GetPointer() ) )
    {
      if ( !reader->LoadScene( document, workingDirectory, storage ) )
      {
        LOG_ERROR << "There were errors while loding scene file " << workingDirectory + "/index.xml. Your data may be corrupted";
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
