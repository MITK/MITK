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

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <mitkSceneFileReader.h>

namespace mitk {

/*
 * This is the module activator for the "SceneSerialization" module.
 */
class SceneSerializationActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context)
  {
    m_SceneDataNodeReader.reset(new mitk::SceneDataNodeReader);
    context->RegisterService(m_SceneDataNodeReader.get());

    m_SceneFileReader = new SceneFileReader();
  }

  void Unload(us::ModuleContext* )
  {
    delete m_SceneFileReader;
  }

private:

  std::auto_ptr<IDataNodeReader> m_SceneDataNodeReader;
  mitk::SceneFileReader* m_SceneFileReader;
};

}

US_EXPORT_MODULE_ACTIVATOR(mitk::SceneSerializationActivator)
