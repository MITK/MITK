/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneReader.h"
#include <tinyxml2.h>

bool mitk::SceneReader::LoadScene(tinyxml2::XMLDocument &document, const std::string &workingDirectory, DataStorage *storage)
{
  // find version node --> note version in some variable
  int fileVersion = 1;
  auto *versionObject = document.FirstChildElement("Version");
  if (versionObject)
  {
    if (versionObject->QueryIntAttribute("FileVersion", &fileVersion) != tinyxml2::XML_SUCCESS)
    {
      MITK_ERROR << "Scene file " << workingDirectory + "/index.xml"
                 << " does not contain version information! Trying version 1 format." << std::endl;
    }
  }

  std::stringstream sceneReaderClassName;
  sceneReaderClassName << "SceneReaderV" << fileVersion;

  std::list<itk::LightObject::Pointer> sceneReaders =
    itk::ObjectFactoryBase::CreateAllInstance(sceneReaderClassName.str().c_str());
  if (sceneReaders.size() < 1)
  {
    MITK_ERROR << "No scene reader found for scene file version " << fileVersion;
  }
  if (sceneReaders.size() > 1)
  {
    MITK_WARN << "Multiple scene readers found for scene file version " << fileVersion
              << ". Using arbitrary first one.";
  }

  for (auto iter = sceneReaders.begin(); iter != sceneReaders.end(); ++iter)
  {
    if (auto *reader = dynamic_cast<SceneReader *>(iter->GetPointer()))
    {
      if (!reader->LoadScene(document, workingDirectory, storage))
      {
        MITK_ERROR << "There were errors while loading scene file "
                   << workingDirectory + "/index.xml. Your data may be corrupted";
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
