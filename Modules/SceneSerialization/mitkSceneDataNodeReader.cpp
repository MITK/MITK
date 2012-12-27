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


#include "mitkSceneDataNodeReader.h"

#include "mitkSceneIO.h"

namespace mitk {

int mitk::SceneDataNodeReader::Read(const std::string &fileName, mitk::DataStorage &storage)
{
  int n = 0;
  if (fileName.substr(fileName.size()-5) == ".mitk")
  {
    SceneIO::Pointer sceneIO = SceneIO::New();
    int oldSize = storage.GetAll()->Size();
    sceneIO->LoadScene(fileName, &storage, false);
    n = storage.GetAll()->Size() - oldSize;
  }
  return n;
}

}
