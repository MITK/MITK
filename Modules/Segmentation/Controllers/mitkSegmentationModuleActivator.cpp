/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkToolManagerProvider.h"

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class SegmentationModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *context) override
    {
      /*register ToolManager provider service*/
      m_ToolManagerProvider = mitk::ToolManagerProvider::New();
      context->RegisterService<mitk::ToolManagerProvider>(m_ToolManagerProvider);
    }

    void Unload(us::ModuleContext *) override {}
  private:
    mitk::ToolManagerProvider::Pointer m_ToolManagerProvider;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::SegmentationModuleActivator)
