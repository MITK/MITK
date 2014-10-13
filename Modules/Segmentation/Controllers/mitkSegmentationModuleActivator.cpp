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

#include <usGetModuleContext.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModule.h>

#include "mitkToolManagerProvider.h"

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class SegmentationModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context)
    {
      /*register ToolManager provider service*/
      m_ToolManagerProvider = mitk::ToolManagerProvider::New();
      context->RegisterService<mitk::ToolManagerProvider>(m_ToolManagerProvider);
    }

    void Unload(us::ModuleContext*)
    {
    }

  private:
    mitk::ToolManagerProvider::Pointer m_ToolManagerProvider;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::SegmentationModuleActivator)
