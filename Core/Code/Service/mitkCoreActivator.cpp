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

#include "mitkRenderingManager.h"

#include <mitkModuleActivator.h>

/*
 * This is the module activator for the "Mitk" module. It registers core services
 * like ...
 */
class MitkCoreActivator : public mitk::ModuleActivator
{
public:

  void Load(mitk::ModuleContext* context)
  {
    //m_RenderingManager = mitk::RenderingManager::New();
    //context->RegisterService<mitk::RenderingManager>(renderingManager.GetPointer());
  }

  void Unload(mitk::ModuleContext* )
  {

  }

private:

  mitk::RenderingManager::Pointer m_RenderingManager;
};

MITK_EXPORT_MODULE_ACTIVATOR(MitkCoreActivator)

