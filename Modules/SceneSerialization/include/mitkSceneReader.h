/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <MitkSceneSerializationExports.h>

#include <tinyxml.h>

#include <itkObjectFactory.h>

#include "mitkDataStorage.h"

namespace mitk
{
  class MITKSCENESERIALIZATION_EXPORT SceneReader : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SceneReader, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      virtual bool LoadScene(TiXmlDocument &document, const std::string &workingDirectory, DataStorage *storage);
  };
}
