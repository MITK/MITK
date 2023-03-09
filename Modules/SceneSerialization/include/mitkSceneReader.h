/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneReader_h
#define mitkSceneReader_h

#include <MitkSceneSerializationExports.h>

#include <itkObjectFactory.h>

#include "mitkDataStorage.h"

namespace tinyxml2
{
  class XMLDocument;
}

namespace mitk
{
  class MITKSCENESERIALIZATION_EXPORT SceneReader : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SceneReader, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    virtual bool LoadScene(tinyxml2::XMLDocument &document, const std::string &workingDirectory, DataStorage *storage);
  };
}

#endif
