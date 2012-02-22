/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
