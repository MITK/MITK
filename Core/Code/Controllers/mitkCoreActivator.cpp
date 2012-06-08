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

#include "mitkRenderingManager.h"
#include "mitkPlanePositionManager.h"
#include "mitkCoreDataNodeReader.h"

#include <mitkModuleActivator.h>

void HandleMicroServicesMessages(mitk::MsgType type, const char* msg)
{
  switch (type)
  {
  case mitk::DebugMsg:
    MITK_DEBUG << msg;
    break;
  case mitk::InfoMsg:
    MITK_INFO << msg;
    break;
  case mitk::WarningMsg:
    MITK_WARN << msg;
    break;
  case mitk::ErrorMsg:
    MITK_ERROR << msg;
    break;
  }
}

/*
 * This is the module activator for the "Mitk" module. It registers core services
 * like ...
 */
class MitkCoreActivator : public mitk::ModuleActivator
{
public:

  void Load(mitk::ModuleContext* context)
  {
    // Handle messages from CppMicroServices
    mitk::installMsgHandler(HandleMicroServicesMessages);

    //m_RenderingManager = mitk::RenderingManager::New();
    //context->RegisterService<mitk::RenderingManager>(renderingManager.GetPointer());
    m_PlanePositionManager = mitk::PlanePositionManagerService::New();
    context->RegisterService<mitk::PlanePositionManagerService>(m_PlanePositionManager);

    m_CoreDataNodeReader = mitk::CoreDataNodeReader::New();
    context->RegisterService<mitk::IDataNodeReader>(m_CoreDataNodeReader);
  }

  void Unload(mitk::ModuleContext* )
  {
    // The mitk::ModuleContext* argument of the Unload() method
    // will always be 0 for the Mitk library. It makes no sense
    // to use it at this stage anyway, since all libraries which
    // know about the module system have already been unloaded.
  }

private:

  //mitk::RenderingManager::Pointer m_RenderingManager;
  mitk::PlanePositionManagerService::Pointer m_PlanePositionManager;
  mitk::CoreDataNodeReader::Pointer m_CoreDataNodeReader;
};

US_EXPORT_MODULE_ACTIVATOR(Mitk, MitkCoreActivator)

