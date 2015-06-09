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

#include <MitkSceneSerializationExports.h>

#include <tinyxml.h>

#include <itkObjectFactory.h>

#include "mitkDataStorage.h"

namespace mitk
{

class MITKSCENESERIALIZATION_EXPORT SceneReader : public itk::Object
{
  public:

    mitkClassMacroItkParent( SceneReader, itk::Object );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual bool LoadScene( TiXmlDocument& document, const std::string& workingDirectory, DataStorage* storage );
};

}

