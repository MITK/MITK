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

