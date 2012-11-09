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

#include <mitkModuleActivator.h>
#include <mitkModuleContext.h>

namespace mitk {

/*
 * This is the module activator for the "SceneSerialization" module.
 */
class SceneSerializationActivator : public ModuleActivator
{
public:

  void Load(mitk::ModuleContext* context)
  {
    m_SceneDataNodeReader = mitk::SceneDataNodeReader::New();
    context->RegisterService<mitk::IDataNodeReader>(m_SceneDataNodeReader);
  }

  void Unload(mitk::ModuleContext* )
  {
  }

private:

  SceneDataNodeReader::Pointer m_SceneDataNodeReader;
};

}

US_EXPORT_MODULE_ACTIVATOR(SceneSerialization, mitk::SceneSerializationActivator)

