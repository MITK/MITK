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
#include "mitkPlanePositionManager.h"

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
      m_PlanePositionManager = mitk::PlanePositionManagerService::New();
      m_PlanePositionManagerReg = context->RegisterService<mitk::PlanePositionManagerService>(m_PlanePositionManager);
  }

  void Unload(mitk::ModuleContext* )
  {
    // The mitk::ModuleContext* argument of the Unload() method
    // will always be 0 for the Mitk library. It makes no sense
    // to use it at this stage anyway, since all libraries which
    // know about the module system have already been unloaded.

      m_PlanePositionManagerReg.Unregister();
      m_PlanePositionManager = 0;

  }

private:

  mitk::RenderingManager::Pointer m_RenderingManager;
  mitk::PlanePositionManagerService::Pointer m_PlanePositionManager;
  mitk::ServiceRegistration m_PlanePositionManagerReg;
};

MITK_EXPORT_MODULE_ACTIVATOR(Mitk, MitkCoreActivator)

